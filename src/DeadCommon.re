/***************************************************************************/
/*                                                                         */
/*   Copyright (c) 2014-2016 LexiFi SAS. All rights reserved.              */
/*                                                                         */
/*   This source code is licensed under the MIT License                    */
/*   found in the LICENSE file at the root of this source tree             */
/*                                                                         */
/***************************************************************************/

let reportUnderscore = ref(false);

let active = Sys.getenv_opt("Global") != None;

let transitive = Sys.getenv_opt("Transitive") != None;

let write = Sys.getenv_opt("Write") != None;

let verbose = Sys.getenv_opt("Verbose") != None;

let deadAnnotation = "dead";

/********   ATTRIBUTES   ********/
module LocSet =
  Set.Make({
    type t = Lexing.position;
    let compare = compare;
  });

module LocHash = {
  include Hashtbl.Make({
    type t = Lexing.position;

    let hash = x => {
      let s = Filename.basename(x.Lexing.pos_fname);
      Hashtbl.hash((x.Lexing.pos_cnum, s));
    };

    let equal = (x: t, y) => x == y;
  });

  let find_set = (h, k) =>
    try(find(h, k)) {
    | Not_found => LocSet.empty
    };

  let add_set = (h, k, v) => {
    let l = find_set(h, k);
    replace(h, k, LocSet.add(v, l));
  };

  let merge_set = (h1, k1, h2, k2) => {
    let l1 = find_set(h1, k1);
    let l2 = find_set(h2, k2);
    replace(h1, k1, LocSet.union(l1, l2));
  };
};

type decs = Hashtbl.t(Lexing.position, string);
let decs: decs = (Hashtbl.create(256): decs); /* all exported value declarations */

let references: LocHash.t(LocSet.t) = (
  LocHash.create(256): LocHash.t(LocSet.t)
); /* all value references */

let fields: Hashtbl.t(string, Lexing.position) = (
  Hashtbl.create(256): Hashtbl.t(string, Lexing.position)
); /* link from fields (record/variant) paths and locations */

let last_loc = ref(Lexing.dummy_pos); /* helper to diagnose occurrences of Location.none in the typedtree */
let current_src = ref("");

let mods: ref(list(string)) = (ref([]): ref(list(string))); /* module path */

let none_ = "_none_";
let include_ = "*include*";

/********   HELPERS   ********/

let getModuleName = fn => fn |> Paths.getModuleName |> ModuleName.toString;

let check_underscore = name => reportUnderscore^ || name.[0] != '_';

let hashtbl_add_to_list = (hashtbl, key, elt) =>
  Hashtbl.add(hashtbl, key, elt);

/* Location printer: `filename:line: ' */
let posToString = (~printCol=false, ~shortFile=false, pos: Lexing.position) => {
  let file = pos.Lexing.pos_fname;
  let line = pos.Lexing.pos_lnum;
  let col = pos.Lexing.pos_cnum - pos.Lexing.pos_bol;
  (shortFile ? file |> Filename.basename : file)
  ++ ":"
  ++ string_of_int(line)
  ++ (printCol ? ":" ++ string_of_int(col) : ": ");
};

/********   PROCESSING  ********/

let export = (~sep=".", path, u, stock: decs, id, loc) => {
  let value =
    String.concat(".", List.rev_map(Ident.name, path))
    ++ sep
    ++ id.Ident.name;

  /* a .cmi file can contain locations from other files.
       For instance:
           module M : Set.S with type elt = int
       will create value definitions whose location is in set.mli
     */
  if (!loc.Location.loc_ghost
      && (
        u == getModuleName(loc.Location.loc_start.Lexing.pos_fname)
        || u === include_
      )
      && check_underscore(id.Ident.name)) {
    hashtbl_add_to_list(stock, loc.Location.loc_start, value);
  };
};

/**** REPORTING ****/

/* Faster than 'List.length l = len' when len < List.length l; same speed otherwise*/
let rec check_length = len =>
  fun
  | [] => len == 0
  | [_, ...l] when len > 0 => check_length(len - 1, l)
  | _ => false;

let pathWithoutHead = path => {
  let rec cutFromNextDot = (s, pos) =>
    if (pos == String.length(s)) {
      s;
    } else if (s.[pos] == '.') {
      String.sub(s, pos + 1, String.length(s) - pos - 1);
    } else {
      cutFromNextDot(s, pos + 1);
    };
  cutFromNextDot(path, 0);
};

/* Keep track of the location of values exported via genType */
module ProcessAnnotations = {
  /* Locations exported to JS */
  let locationsAnnotatedWithGenType = LocHash.create(1);
  let locationsAnnotatedDead = LocHash.create(1);

  let isAnnotatedDead = loc => LocHash.mem(locationsAnnotatedDead, loc);

  let isAnnotatedGentypeOrDead = loc =>
    LocHash.mem(locationsAnnotatedWithGenType, loc) || isAnnotatedDead(loc);

  let locAnnotatedWithGenType = (loc: Lexing.position) => {
    LocHash.replace(locationsAnnotatedWithGenType, loc, ());
  };

  let locAnnotatedDead = (loc: Lexing.position) => {
    LocHash.replace(locationsAnnotatedDead, loc, ());
  };

  let processAttributes = (~ignoreInterface, ~loc, attributes) => {
    if (attributes |> Annotation.hasGenTypeAnnotation(~ignoreInterface)) {
      loc |> locAnnotatedWithGenType;
    };
    if (attributes
        |> Annotation.getAttributePayload((==)(deadAnnotation)) != None) {
      loc |> locAnnotatedDead;
    };
  };

  let collectExportLocations = (~ignoreInterface) => {
    let super = Tast_mapper.default;
    let value_binding =
        (
          self,
          {vb_attributes, vb_pat} as value_binding: Typedtree.value_binding,
        ) => {
      switch (vb_pat.pat_desc) {
      | Tpat_var(id, pLoc) =>
        vb_attributes
        |> processAttributes(~ignoreInterface, ~loc=pLoc.loc.loc_start)

      | _ => ()
      };
      super.value_binding(self, value_binding);
    };
    let value_description =
        (
          self,
          {val_attributes, val_id, val_loc} as value_description: Typedtree.value_description,
        ) => {
      val_attributes
      |> processAttributes(~ignoreInterface, ~loc=val_loc.loc_start);
      super.value_description(self, value_description);
    };
    {...super, value_binding, value_description};
  };

  let structure = structure => {
    let ignoreInterface = ref(false);
    let collectExportLocations = collectExportLocations(~ignoreInterface);
    structure
    |> collectExportLocations.structure(collectExportLocations)
    |> ignore;
  };
  let signature = signature => {
    let ignoreInterface = ref(false);
    let collectExportLocations = collectExportLocations(~ignoreInterface);
    signature
    |> collectExportLocations.signature(collectExportLocations)
    |> ignore;
  };
};

type item = {
  pos: Lexing.position,
  path: string,
};

let compareItems = ({path: path1, pos: pos1}, {path: path2, pos: pos2}) =>
  compare((pos1, path1), (pos2, path2));

let report = (~useDead=false, ~onItem, decs: decs) => {
  let dontReportDead = pos =>
    useDead && ProcessAnnotations.isAnnotatedGentypeOrDead(pos);
  let folder = (pos, path, items) => {
    switch (pos |> LocHash.find_set(references)) {
    | referencesToLoc when !(pos |> dontReportDead) =>
      if (referencesToLoc |> LocSet.cardinal == 0) {
        [{pos, path: pathWithoutHead(path)}, ...items];
      } else {
        if (verbose) {
          let refsString =
            referencesToLoc
            |> LocSet.elements
            |> List.map(posToString(~printCol=true, ~shortFile=true))
            |> String.concat(", ");
          GenTypeCommon.logItem(
            "%s: %d references (%s)\n",
            path,
            referencesToLoc |> LocSet.cardinal,
            refsString,
          );
        };
        items;
      }
    | _ => items
    | exception Not_found => items
    };
  };

  Hashtbl.fold(folder, decs, [])
  |> List.fast_sort(compareItems)
  |> List.iter(onItem);
};