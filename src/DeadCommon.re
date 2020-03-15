/* Adapted from https://github.com/LexiFi/dead_code_analyzer */

// Turn on the main dead code analysis
let dce = ref(true);

// Turn on type analysis
let analyzeTypes = ref(true);

let analyzeTermination = ref(false);

let analyzeExternals = false;

let transitive = true;

let verbose = Sys.getenv_opt("Debug") != None;

let checkPrefix = prefix_ => {
  let prefix =
    GenTypeCommon.projectRoot^ == ""
      ? prefix_ : Filename.concat(GenTypeCommon.projectRoot^, prefix_);
  let prefixLen = prefix |> String.length;
  sourceDir =>
    String.length(sourceDir) >= prefixLen
    && String.sub(sourceDir, 0, prefixLen) == prefix;
};

let rec checkSub = (s1, s2, n) =>
  n <= 0 || s1.[n] == s2.[n] && checkSub(s1, s2, n - 1);
let fileIsImplementationOf = (s1, s2) => {
  let n1 = String.length(s1)
  and n2 = String.length(s2);
  n2 == n1 + 1 && checkSub(s1, s2, n1 - 1);
};

// Whitelist=prefix only report on source dirs with the given prefix
let whitelistSourceDir =
  lazy(
    {
      switch (Sys.getenv_opt("Whitelist")) {
      | None => (_sourceDir => true)
      | Some(prefix) => checkPrefix(prefix)
      };
    }
  );

let posInWhitelist = (pos: Lexing.position) => {
  pos.pos_fname |> Lazy.force(whitelistSourceDir);
};

// Blacklist=prefix don't report on source dirs with the given prefix
let blacklistSourceDir =
  lazy(
    {
      switch (Sys.getenv_opt("Blacklist")) {
      | None => (_sourceDir => false)
      | Some(prefix) => checkPrefix(prefix)
      };
    }
  );

let posInBlacklist = (pos: Lexing.position) => {
  pos.pos_fname |> Lazy.force(blacklistSourceDir);
};

let write = Sys.getenv_opt("Write") != None;

let deadAnnotation = "dead";
let liveAnnotation = "live";

type declKind =
  | RecordLabel
  | VariantCase
  | Value;

/* Location printer: `filename:line: ' */
let posToString = (~printCol=true, ~shortFile=true, pos: Lexing.position) => {
  let file = pos.Lexing.pos_fname;
  let line = pos.Lexing.pos_lnum;
  let col = pos.Lexing.pos_cnum - pos.Lexing.pos_bol;
  (shortFile ? file |> Filename.basename : file)
  ++ ":"
  ++ string_of_int(line)
  ++ (printCol ? ":" ++ string_of_int(col) : ": ");
};

let posIsReason = (pos: Lexing.position) =>
  Filename.check_suffix(pos.pos_fname, ".re")
  || Filename.check_suffix(pos.pos_fname, ".rei");

/********   ATTRIBUTES   ********/
module PosSet =
  Set.Make({
    type t = Lexing.position;
    let compare = compare;
  });

module PosHash = {
  include Hashtbl.Make({
    type t = Lexing.position;

    let hash = x => {
      let s = Filename.basename(x.Lexing.pos_fname);
      Hashtbl.hash((x.Lexing.pos_cnum, s));
    };

    let equal = (x: t, y) => x == y;
  });

  let findSet = (h, k) =>
    try(find(h, k)) {
    | Not_found => PosSet.empty
    };

  let addSet = (h, k, v) => {
    let set = findSet(h, k);
    replace(h, k, PosSet.add(v, set));
  };

  let mergeSet = (~isType, ~from, ~to_, table) => {
    let set1 = findSet(table, to_);
    let set2 = findSet(table, from);
    let setUnion = PosSet.union(set1, set2);
    replace(table, to_, setUnion);
    if (verbose) {
      Log_.item(
        "%smergeSet %s --> %s (%d new items)@.",
        isType ? "[type] " : "",
        from |> posToString,
        to_ |> posToString,
        PosSet.cardinal(setUnion) - PosSet.cardinal(set1),
      );
    };
  };
};

module FileSet = Set.Make(String);

module FileHash = {
  include Hashtbl.Make({
    type t = string;

    let hash = (x: t) => Hashtbl.hash(x);

    let equal = (x: t, y) => x == y;
  });

  let findSet = (table, key) =>
    try(find(table, key)) {
    | Not_found => FileSet.empty
    };

  let addFile = (table, key) => {
    let set = findSet(table, key);
    replace(table, key, set);
  };

  let addSet = (table, key, value) => {
    let set = findSet(table, key);
    replace(table, key, FileSet.add(value, set));
  };
};

type path = list(string);
type decls = Hashtbl.t(Lexing.position, (path, declKind, Lexing.position));
let decls: decls = Hashtbl.create(256); /* all exported declarations */
type recInfo = {
  name: string,
  recSet: PosSet.t,
  mutable resolved: bool,
};
let recursiveDecls: PosHash.t(recInfo) = PosHash.create(256); /* all recursive declarations */

let valueReferences: PosHash.t(PosSet.t) = PosHash.create(256); /* all value references */
let typeReferences: PosHash.t(PosSet.t) = PosHash.create(256); /* all type references */

let fileReferences: FileHash.t(FileSet.t) = FileHash.create(256); /* references across files */

let fields: Hashtbl.t(string, Lexing.position) = (
  Hashtbl.create(256): Hashtbl.t(string, Lexing.position)
); /* link from fields (record/variant) paths and locations */

let currentSrc = ref("");
let currentModuleName = ref("");
let currentBindings = ref(PosSet.empty);
let lastBinding = ref(Lexing.dummy_pos);
let getLastBinding = () => lastBinding^;

/* Keep track of the module path while traversing with Tast_mapper */
let currentModulePath: ref(path) = ref([]);

let none_ = "_none_";
let include_ = "*include*";

/********   HELPERS   ********/

let addValueReference = (~addFileReference, posDeclaration, posUsage) => {
  let lastBinding = getLastBinding();
  let posUsage =
    !transitive || lastBinding == Lexing.dummy_pos ? posUsage : lastBinding;
  let posReRouted =
    // if posDeclaration is recursive toghether with {x,y,z},
    // and there exists y in {x,y,z} which is a current binding
    // then rerout the binding to y --> posDeclaration
    switch (PosHash.find_opt(recursiveDecls, posDeclaration)) {
    | Some({name, recSet}) =>
      switch (
        PosSet.find_first_opt(
          pos => PosSet.mem(pos, currentBindings^),
          recSet,
        )
      ) {
      | Some(posReRouted) =>
        if (verbose && posReRouted != posUsage) {
          Log_.item(
            "recursiveCall to %s: %s rerouted to %s@.",
            name,
            posUsage |> posToString,
            posReRouted |> posToString,
          );
        };
        posReRouted;
      | None => posUsage
      }
    | None => posUsage
    };
  if (verbose) {
    Log_.item(
      "addValueReference %s --> %s@.",
      posReRouted |> posToString,
      posDeclaration |> posToString,
    );
  };
  PosHash.addSet(valueReferences, posDeclaration, posReRouted);
  if (addFileReference
      && posDeclaration.pos_fname != none_
      && posReRouted.pos_fname != none_
      && posReRouted.pos_fname != posDeclaration.pos_fname) {
    FileHash.addSet(
      fileReferences,
      posReRouted.pos_fname,
      posDeclaration.pos_fname,
    );
  };
};

let iterFilesFromRootsToLeaves = iterFun => {
  /* For each file, the number of incoming references */
  let inverseReferences: Hashtbl.t(string, int) = Hashtbl.create(1);
  /* For each number of incoming references, the files */
  let referencesByNumber: Hashtbl.t(int, FileSet.t) = Hashtbl.create(1);

  let getNum = fileName =>
    try(Hashtbl.find(inverseReferences, fileName)) {
    | Not_found => 0
    };

  let getSet = num =>
    try(Hashtbl.find(referencesByNumber, num)) {
    | Not_found => FileSet.empty
    };

  let addIncomingEdge = fileName => {
    let oldNum = getNum(fileName);
    let newNum = oldNum + 1;
    let oldSetAtNum = getSet(oldNum);
    let newSetAtNum = FileSet.remove(fileName, oldSetAtNum);
    let oldSetAtNewNum = getSet(newNum);
    let newSetAtNewNum = FileSet.add(fileName, oldSetAtNewNum);
    Hashtbl.replace(inverseReferences, fileName, newNum);
    Hashtbl.replace(referencesByNumber, oldNum, newSetAtNum);
    Hashtbl.replace(referencesByNumber, newNum, newSetAtNewNum);
  };

  let removeIncomingEdge = fileName => {
    let oldNum = getNum(fileName);
    let newNum = oldNum - 1;
    let oldSetAtNum = getSet(oldNum);
    let newSetAtNum = FileSet.remove(fileName, oldSetAtNum);
    let oldSetAtNewNum = getSet(newNum);
    let newSetAtNewNum = FileSet.add(fileName, oldSetAtNewNum);
    Hashtbl.replace(inverseReferences, fileName, newNum);
    Hashtbl.replace(referencesByNumber, oldNum, newSetAtNum);
    Hashtbl.replace(referencesByNumber, newNum, newSetAtNewNum);
  };

  let isSourceFile = fileName => FileHash.mem(fileReferences, fileName);

  let addEdge = (fromFile, toFile) =>
    if (isSourceFile(fromFile)) {
      addIncomingEdge(toFile);
    };

  let removeEdge = (fromFile, toFile) =>
    if (isSourceFile(fromFile)) {
      removeIncomingEdge(toFile);
    };

  fileReferences
  |> FileHash.iter((fromFile, set) => {
       if (getNum(fromFile) == 0) {
         Hashtbl.replace(
           referencesByNumber,
           0,
           FileSet.add(fromFile, getSet(0)),
         );
       };
       set |> FileSet.iter(toFile => {addEdge(fromFile, toFile)});
     });

  while (getSet(0) != FileSet.empty) {
    let filesWithNoIncomingReferences = getSet(0);
    Hashtbl.remove(referencesByNumber, 0);
    filesWithNoIncomingReferences
    |> FileSet.iter(fileName => {
         iterFun(fileName);
         let references =
           try(FileHash.find(fileReferences, fileName)) {
           | Not_found => FileSet.empty
           };
         references |> FileSet.iter(toFile => removeEdge(fileName, toFile));
       });
  };
  // Process any remaining items in case of circular references
  referencesByNumber
  |> Hashtbl.iter((_num, set) =>
       if (FileSet.is_empty(set)) {
         ();
       } else {
         set
         |> FileSet.iter(fileName => {
              let pos = {...Lexing.dummy_pos, pos_fname: fileName};
              let loc = {...Location.none, loc_start: pos, loc_end: pos};
              Log_.info(~loc, ~name="Warning Dead Analysis Cycle", (ppf, ()) =>
                Format.fprintf(
                  ppf,
                  "Results for %s could be inaccurate because of circular references",
                  fileName,
                )
              );
              iterFun(fileName);
            });
       }
     );
};

/********   PROCESSING  ********/

let pathToString = path => path |> List.rev |> String.concat(".");

let pathWithoutHead = path => {
  path |> List.rev |> List.tl |> String.concat(".");
};

let annotateAtEnd = (~pos) => !posIsReason(pos);

let addDeclaration = (~declKind, ~path, ~loc: Location.t, ~name) => {
  let pos = loc.loc_start;
  let posEnd = loc.loc_end;
  let posAnnotation = annotateAtEnd(~pos) ? posEnd : pos;

  /* a .cmi file can contain locations from other files.
       For instance:
           module M : Set.S with type elt = int
       will create value definitions whose location is in set.mli
     */
  if (!loc.loc_ghost
      && (currentSrc^ == pos.pos_fname || currentModuleName^ === include_)) {
    if (verbose) {
      Log_.item(
        "%saddDeclaration %s %s@.",
        declKind != Value ? "[type] " : "",
        name,
        pos |> posToString,
      );
    };

    Hashtbl.add(decls, pos, ([name, ...path], declKind, posAnnotation));
  };
};

/**** REPORTING ****/

/* Keep track of the location of values annotated @genType or @dead */
module ProcessDeadAnnotations = {
  type annotatedAs =
    | GenType
    | Dead
    | Live;

  let positionsAnnotated = PosHash.create(1);

  let isAnnotatedDead = pos =>
    PosHash.find_opt(positionsAnnotated, pos) == Some(Dead);

  let isAnnotatedLive = pos =>
    PosHash.find_opt(positionsAnnotated, pos) == Some(Live);

  let isAnnotatedGenTypeOrDead = pos =>
    switch (PosHash.find_opt(positionsAnnotated, pos)) {
    | Some(Dead | GenType) => true
    | Some(Live)
    | None => false
    };

  let annotateGenType = (pos: Lexing.position) => {
    PosHash.replace(positionsAnnotated, pos, GenType);
  };

  let annotateDead = (pos: Lexing.position) => {
    PosHash.replace(positionsAnnotated, pos, Dead);
  };

  let annotateLive = (pos: Lexing.position) => {
    PosHash.replace(positionsAnnotated, pos, Live);
  };

  let processAttributes = (~pos, attributes) => {
    if (attributes
        |> Annotation.getAttributePayload(
             Annotation.tagIsOneOfTheGenTypeAnnotations,
           )
        != None) {
      pos |> annotateGenType;
    };
    if (attributes
        |> Annotation.getAttributePayload((==)(deadAnnotation)) != None) {
      pos |> annotateDead;
    } else if (attributes
               |> Annotation.getAttributePayload((==)(liveAnnotation))
               != None) {
      pos |> annotateLive;
    };
  };

  let collectExportLocations = () => {
    let super = Tast_mapper.default;
    let value_binding =
        (
          self,
          {vb_attributes, vb_pat} as value_binding: Typedtree.value_binding,
        ) => {
      switch (vb_pat.pat_desc) {
      | Tpat_var(_id, pLoc) =>
        vb_attributes |> processAttributes(~pos=pLoc.loc.loc_start)

      | _ => ()
      };
      super.value_binding(self, value_binding);
    };
    let type_kind = (self, typeKind: Typedtree.type_kind) => {
      switch (typeKind) {
      | Ttype_record(labelDeclarations) =>
        labelDeclarations
        |> List.iter(({ld_attributes, ld_loc}: Typedtree.label_declaration) =>
             ld_attributes |> processAttributes(~pos=ld_loc.loc_start)
           )
      | Ttype_variant(constructorDeclarations) =>
        constructorDeclarations
        |> List.iter(
             ({cd_attributes, cd_loc}: Typedtree.constructor_declaration) =>
             cd_attributes |> processAttributes(~pos=cd_loc.loc_start)
           )
      | _ => ()
      };
      super.type_kind(self, typeKind);
    };
    let value_description =
        (
          self,
          {val_attributes, val_val} as value_description: Typedtree.value_description,
        ) => {
      val_attributes |> processAttributes(~pos=val_val.val_loc.loc_start);
      super.value_description(self, value_description);
    };
    {...super, type_kind, value_binding, value_description};
  };

  let structure = structure => {
    let collectExportLocations = collectExportLocations();
    structure
    |> collectExportLocations.structure(collectExportLocations)
    |> ignore;
  };
  let signature = signature => {
    let collectExportLocations = collectExportLocations();
    signature
    |> collectExportLocations.signature(collectExportLocations)
    |> ignore;
  };
};

type item = {
  declKind,
  path,
  pos: Lexing.position,
  posAnnotation: Lexing.position,
};

module WriteDeadAnnotations = {
  type line = {
    mutable items: list(item),
    original: string,
  };

  let rec lineToString = ({original, items}) => {
    switch (items) {
    | [] => original
    | [{declKind, path, pos, posAnnotation}, ...otherItems] =>
      let isReason = posIsReason(pos);
      let annotationStr =
        (isReason ? "" : " ")
        ++ "["
        ++ (isReason || declKind != Value ? "@" : "@@")
        ++ deadAnnotation
        ++ " \""
        ++ (path |> pathWithoutHead)
        ++ "\"] ";
      let col = posAnnotation.pos_cnum - posAnnotation.pos_bol;
      let originalLen = String.length(original);
      {
        original:
          if (String.length(original) >= col && col > 0) {
            let original1 = String.sub(original, 0, col);
            let original2 = String.sub(original, col, originalLen - col);
            original1 ++ annotationStr ++ original2;
          } else {
            isReason ? annotationStr ++ original : original ++ annotationStr;
          },
        items: otherItems,
      }
      |> lineToString;
    };
  };

  let currentFile = ref("");
  let currentFileLines: ref(array(line)) = ref([||]);

  let readFile = fileName => {
    let channel = open_in(fileName);
    let lines = ref([]);
    let rec loop = () => {
      let line = {original: input_line(channel), items: []};
      lines := [line, ...lines^];
      loop();
    };
    try(loop()) {
    | End_of_file =>
      close_in(channel);
      lines^ |> List.rev |> Array.of_list;
    };
  };

  let writeFile = (fileName, lines) =>
    if (fileName != "" && write) {
      let channel = open_out(fileName);
      let lastLine = Array.length(lines);
      lines
      |> Array.iteri((n, line) => {
           output_string(channel, line |> lineToString);
           if (n < lastLine - 1) {
             output_char(channel, '\n');
           };
         });
      close_out(channel);
    };

  let onDeadItem = (~ppf, item) => {
    let fileName = item.pos.pos_fname;
    if (Sys.file_exists(fileName)) {
      if (fileName != currentFile^) {
        writeFile(currentFile^, currentFileLines^);
        currentFile := fileName;
        currentFileLines := readFile(fileName);
      };

      let indexInLines = item.posAnnotation.pos_lnum - 1;

      if (indexInLines < Array.length(currentFileLines^)) {
        let line = currentFileLines^[indexInLines];
        line.items = [item, ...line.items];
        Format.fprintf(
          ppf,
          "  <-- line %d@.  %s@.",
          item.pos.pos_lnum,
          line |> lineToString,
        );
      } else {
        Format.fprintf(ppf, "  <-- Can't find line %d@.", item.pos.pos_lnum);
      };
    } else {
      Format.fprintf(ppf, "  <-- can't find file@.");
    };
  };

  let write = () => writeFile(currentFile^, currentFileLines^);
};

let rec resolveRecursiveRefs = (~refsTodo, ~refsBeingResolved, pos) => {
  switch (PosHash.find_opt(recursiveDecls, pos)) {
  | None => refsTodo
  | Some({name, resolved: true}) =>
    Log_.item("XXX %s already resolved@.", name);
    refsTodo;
  | Some(recInfo) when PosSet.mem(pos, refsBeingResolved) =>
    Log_.item(
      "XXX %s is being resolved already: assume no new dependencies@.",
      recInfo.name,
    );
    PosSet.empty;
  | Some({name, recSet} as recInfo) =>
    Log_.item("@.XXX resolving %s@.", name);
    let unresolved = ref(false);
    let newRefsTodo =
      refsTodo
      |> PosSet.filter(x =>
           if (x == pos) {
             Log_.item("XXX %s ignoring reference to self@.", name);
             false;
           } else if (!PosSet.mem(x, recSet)) {
             Log_.item(
               "XXX %s: one dependency is not mutually recursive@.",
               name,
             );
             true;
           } else {
             let xRecSet = PosHash.find(recursiveDecls, x);
             let xRefsTodo = PosHash.findSet(valueReferences, x);
             if (xRecSet.resolved) {
               !ProcessDeadAnnotations.isAnnotatedDead(x)
               && !PosSet.is_empty(xRefsTodo);
             } else {
               let xNewRefsTodo =
                 x
                 |> resolveRecursiveRefs(
                      ~refsTodo=xRefsTodo,
                      ~refsBeingResolved=PosSet.add(pos, refsBeingResolved),
                    );
               if (!xRecSet.resolved) {
                 unresolved := true;
               };
               !PosSet.is_empty(xNewRefsTodo);
             };
           }
         );
    if (! unresolved^ || PosSet.is_empty(refsBeingResolved)) {
      recInfo.resolved = true;
      if (PosSet.is_empty(newRefsTodo)) {
        pos |> ProcessDeadAnnotations.annotateDead;
      };
      Log_.item(
        "XXX %s resolved with %d dependencies@.",
        name,
        newRefsTodo |> PosSet.cardinal,
      );
    } else {
      Log_.item("XXX %s still unresolved@.", name);
    };
    newRefsTodo;
  };
};

let reportDead = (~onDeadCode) => {
  let dontReportDead = pos =>
    ProcessDeadAnnotations.isAnnotatedGenTypeOrDead(pos);

  let iterOrderedItem = (items, {declKind, pos, path} as item) => {
    switch (
      pos
      |> PosHash.findSet(declKind == Value ? valueReferences : typeReferences)
    ) {
    | referencesToLoc when !(pos |> dontReportDead) =>
      let referencesToLoc =
        resolveRecursiveRefs(
          ~refsBeingResolved=PosSet.empty,
          ~refsTodo=referencesToLoc,
          pos,
        );
      let liveReferences =
        referencesToLoc
        |> PosSet.filter(p => !ProcessDeadAnnotations.isAnnotatedDead(p));
      if (liveReferences
          |> PosSet.cardinal == 0
          && !ProcessDeadAnnotations.isAnnotatedLive(pos)
          && posInWhitelist(pos)
          && !posInBlacklist(pos)) {
        if (transitive) {
          pos |> ProcessDeadAnnotations.annotateDead;
        };
        [{...item, path}, ...items];
      } else {
        if (verbose) {
          let refsString =
            referencesToLoc
            |> PosSet.elements
            |> List.map(posToString)
            |> String.concat(", ");
          Log_.item(
            "%s%s: %d references (%s)@.",
            declKind != Value ? "[type] " : "",
            path |> pathToString,
            referencesToLoc |> PosSet.cardinal,
            refsString,
          );
        };
        items;
      };
    | _ => items
    | exception Not_found => items
    };
  };

  if (verbose) {
    Log_.item("@.File References@.@.");
    fileReferences
    |> FileHash.iter((file, files) =>
         Log_.item(
           "%s -->> %s@.",
           file |> Filename.basename,
           files
           |> FileSet.elements
           |> List.map(Filename.basename)
           |> String.concat(", "),
         )
       );
  };

  let items =
    Hashtbl.fold(
      (pos, (path, declKind, posAnnotation), items) =>
        [{declKind, path, pos, posAnnotation}, ...items],
      decls,
      [],
    );

  let orderedFiles = Hashtbl.create(256);
  iterFilesFromRootsToLeaves(
    {
      let current = ref(0);
      fileName => {
        incr(current);
        Hashtbl.add(orderedFiles, fileName, current^);
      };
    },
  );
  let compareItemsUsingDependencies =
      (
        {
          declKind: kind1,
          path: path1,
          pos: {
            pos_fname: fname1,
            pos_lnum: lnum1,
            pos_bol: bol1,
            pos_cnum: cnum1,
          },
        },
        {
          declKind: kind2,
          path: path2,
          pos: {
            pos_fname: fname2,
            pos_lnum: lnum2,
            pos_bol: bol2,
            pos_cnum: cnum2,
          },
        },
      ) => {
    let findPosition = fn => Hashtbl.find(orderedFiles, fn);

    let pathIsImplementationOf = (path1, path2) =>
      switch (path1, path2) {
      | ([name1, ...restPath1], [name2, ...restPath2]) =>
        name1.[0] == '+'
        && name2.[0] != '+'
        && List.length(restPath1) > 1
        && restPath1 == restPath2
      | ([], _)
      | (_, []) => false
      };

    /* From the root of the file dependency DAG to the leaves.
       From the bottom of the file to the top. */
    let (position1, position2) = (
      fname1 |> findPosition,
      fname2 |> findPosition,
    );
    let (p1, p2) =
      pathIsImplementationOf(path1, path2)
        ? (1, 0) : pathIsImplementationOf(path2, path1) ? (0, 1) : (0, 0);
    compare(
      (position1, p1, lnum2, bol2, cnum2, kind1),
      (position2, p2, lnum1, bol1, cnum1, kind2),
    );
  };

  items
  |> List.fast_sort(compareItemsUsingDependencies)  /* analyze in reverse order */
  |> List.fold_left(iterOrderedItem, [])
  |> List.iter(item => item |> onDeadCode);
};