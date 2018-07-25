open GenFlowCommon;

type converter =
  | Unit
  | Identity
  | OptionalArgument(converter)
  | Option(converter)
  | Fn((list((NamedArgs.label, converter)), converter));

type dependency =
  /* Import a type that we expect to also be genFlow'd. */
  | TypeAtPath(Path.t)
  /* Imports a JS type (typeName, importAs, jsModuleName) */
  | JSTypeFromModule(string, string, string)
  /* (type variable name, unique type id) */
  | FreeTypeVariable(string, int);

type convertableFlowType = (converter, Flow.typ);

type conversionPlan = (list(dependency), convertableFlowType);

type import =
  | ImportComment(string)
  | ImportAsFrom(string, string, string);

type exportType = {
  opaque: bool,
  typeParams: list(Flow.typ),
  name: string,
  flowType: Flow.typ,
};

type exportUnionType = {
  typeParams: list(Flow.typ),
  leafTypes: list(Flow.typ),
  name: string,
};

type t =
  | ImportType(import)
  | ExportType(exportType)
  | ExportUnionType(exportUnionType)
  | FlowTypeBinding(string, Flow.typ)
  | ValueBinding(ModuleName.t, string, converter)
  | ConstructorBinding(Flow.typ, list(convertableFlowType), string, int)
  | ComponentBinding(
      ModuleName.t,
      option(Flow.typ),
      Ident.t,
      converter,
      string,
    );

type genFlowKind =
  | NoGenFlow
  | GenFlow
  | GenFlowOpaque;

/*
  When reading the data structures: There are structures from the `Typedtree`
  module which is the typed AST, and that AST references types from the module
  `Types` which represent the result of type checking.

  - `Typedtree` usually has record fields of the form `typ_x`
  - `Typedtree` usually has variants of the form Ttype_foo
  - `Types` usually has record fields of the form `type_x`
  - `Types` usually has variants of the form Type_foo
  - types (not fields or variant names) defined in both `Typedtree` and
  `Types` begin with the prefix `type_foo`

  There is a lot of redundancy between the two trees.

  Typedtree Module:                                Types Module:
  ----------------------                           ----------------------
  type_declaration = {                          -->type type_declaration = {
                                               /     type_params: type_expr list,
    typ_id: Ident.t,                          /      type_arity: int,
    typ_name: string loc,                    /       type_kind: type_kind,
    typ_params: (core_type * variance) list,/        type_private: private_flag,
    typ_type: Types.type_declaration,  ----/         type_manifest: type_expr option,
    typ_cstrs:(core_type*core_type*Location.t)list   type_variance: Variance.t list,
    typ_kind: type_kind,                             (* covariant, contravariant, weakly contravariant, injective *)
    typ_private: private_flag,                       type_is_newtype: bool,
    typ_manifest: core_type option,                  type_expansion_scope: int option,
    typ_loc: Location.t,                             type_loc: Location.t,
    typ_attributes: attributes                       type_attributes: Parsetree.attributes,
    typ_attributes: attributes                       type_immediate: bool, (* true iff type should not be a pointer *)
  }                                                  type_unboxed: unboxed_status,
                                                   }

  Typedtree Module:                                Types Module:
  ----------------------                           ----------------------
  type type_kind =                                 type type_kind =
  | Ttype_abstract                                 | Type_abstract
  | Ttype_variant of constructor_declaration list  | Type_record of label_declaration list * record_representation
  | Ttype_record of label_declaration list         | Type_variant of constructor_declaration list
  | Ttype_open                                     | Type_open

  Typedtree Module:                                Types Module:
  ----------------------                           ----------------------
  type constructor_declaration = {                 type constructor_declaration = {
    cd_id: Ident.t,                                   cd_id: Ident.t,
    cd_name: string loc,                              cd_args: constructor_arguments,
    cd_args: constructor_arguments,                   cd_res: type_expr option,
    cd_res: core_type option,                         cd_loc: Location.t,
    cd_loc: Location.t,                               cd_attributes: Parsetree.attributes,
    cd_attributes: attributes                      }
  }

  type constructor_arguments =                     type constructor_arguments =
  | Cstr_tuple of core_type list                    | Cstr_tuple of type_expr list
  | Cstr_record of label_declaration list           | Cstr_record of label_declaration list


                        This pointer is mutated
  Typedtree Module:    with the result of type     Types Module:
  -------------        checking once complete!     -------------
  type core_type = {                          ---> type type_expr = {
    mutable ctyp_desc : core_type_desc;      /       mutable desc: type_desc;
    mutable ctyp_type : type_expr; ---------/        mutable level: int;
    ctyp_env : Env.t;                                mutable scope: int option;
    ctyp_loc : Location.t;                           id: int
    ctyp_attributes: attributes;                   }
  }

  type core_type_desc =                            type_desc =
  | Ttyp_any                                        | Tvar of string option
  | Ttyp_var of string                              | Tarrow of arg_label * type_expr * type_expr * commutable
  | Ttyp_arrow of arg_label*core_type*core_type     | Ttuple of type_expr list
  | Ttyp_tuple of core_type list                    | Tconstr of Path.t * type_expr list * abbrev_memo ref
  | Ttyp_constr of                                  | Tobject of type_expr * (Path.t * type_expr list) option ref
    Path.t*Longident.t loc*core_type list           | Tfield of string * field_kind * type_expr * type_expr
  | Ttyp_object of object_field list * closed_flag  | Tnil
  | Ttyp_class of                                   | Tlink of type_expr
     Path.t * Longident.t loc * core_type list      | Tsubst of type_expr         (* for copying *)
  | Ttyp_alias of core_type * string                | Tvariant of row_desc
  | Ttyp_variant of                                 | Tunivar of string option
     row_field list*closed_flag*label list option   | Tpoly of type_expr * type_expr list
  | Ttyp_poly of string list * core_type
  | Ttyp_package of package_type

  Typedtree.type_declaration will be something like: {
    type_type: {
      type_params: ...
      type_kind: type_kind =
         | Type_abstract
         | Type_record of label_declaration list  * record_representation
         | Type_variant of constructor_declaration list
         | Type_open
      type_manifest: Some({
        mutable desc: type_desc =
           | Tvar of string option
           | Tarrow of arg_label * type_expr * type_expr * commutable
           | Ttuple of type_expr list
           | Tconstr of Path.t * type_expr list * abbrev_memo ref
           | Tobject of type_expr * (Path.t * type_expr list) option ref
           | Tfield of string * field_kind * type_expr * type_expr
           | Tnil
           | Tlink of type_expr
           | Tsubst of type_expr         (* for copying *)
           | Tvariant of row_desc
           | Tunivar of string option
           | Tpoly of type_expr * type_expr list
      }),
      type_arity: int;
      type_private: private_flag;
      type_variance: Variance.t list;
      type_is_newtype: bool;
      type_expansion_scope: int option;
      type_loc: Location.t;
      type_attributes: Parsetree.attributes;
      type_immediate: bool; (* true iff type should not be a pointer *)
      type_unboxed: unboxed_status;
    }
  }
 */

let rec hasAttribute = (searchText, attributes) =>
  switch (attributes) {
  | [] => false
  | [({Asttypes.txt}, _), ...tl] when txt == searchText => true
  | [hd, ...tl] => hasAttribute(searchText, tl)
  };

let getGenFlowKind = attrs =>
  if (hasAttribute(tagSearch, attrs)) {
    GenFlow;
  } else if (hasAttribute(tagSearchOpaque, attrs)) {
    GenFlowOpaque;
  } else {
    NoGenFlow;
  };

let exportTypeToString = ({opaque, typeParams, name, flowType}) =>
  "export"
  ++ (opaque ? " opaque " : " ")
  ++ "type "
  ++ name
  ++ Flow.genericsString(List.map(Flow.render, typeParams))
  ++ " = "
  ++ Flow.render(flowType)
  ++ (opaque ? " // Reason type already checked. Making it opaque" : "");

let codeItemForType = (~opaque, typeParams, name, flowType) =>
  ExportType({opaque, typeParams, name, flowType});

let exportUnionTypeToString = ({typeParams, leafTypes, name}) =>
  "export type "
  ++ name
  ++ Flow.genericsString(List.map(Flow.render, typeParams))
  ++ " =\n  | "
  ++ String.concat("\n  | ", List.map(Flow.render, leafTypes));

let rec removeOption = (label, typ) =>
  Types.(
    switch (typ.desc) {
    | Tconstr(Path.Pident(id), [t], _)
        /* This has a different representation in 4.03+ */
        when Ident.name(id) == "option" && label != "" && label.[0] == '?' =>
      Some((String.sub(label, 1, String.length(label) - 1), t))
    | Tconstr(Pdot(Path.Pident(nameSpace), id, _), [t], _)
        /* This has a different representation in 4.03+ */
        when
          Ident.name(nameSpace) == "FB"
          && id == "option"
          && label != ""
          && label.[0] == '?' =>
      Some((String.sub(label, 1, String.length(label) - 1), t))
    | Tlink(t) => removeOption(label, t)
    | _ => None
    }
  );

/**
 * Turns
 *
 *     [(x, (a, b)), (y, (c, d)), ...]
 *
 * Into:
 *
 *     (
 *       [(x, a), (y, c), ...],
 *       [(x, b), (y, d), ...]
 *     )
 */
let rec distributeSplitRev_ = (revLeftSoFar, revRightSoFar, lst) =>
  switch (lst) {
  | [] => (revLeftSoFar, revRightSoFar)
  | [(toDistribute, (ontoA, ontoB)), ...tl] =>
    distributeSplitRev_(
      [(toDistribute, ontoA), ...revLeftSoFar],
      [(toDistribute, ontoB), ...revRightSoFar],
      tl,
    )
  };

let distributeSplitRev = lst => distributeSplitRev_([], [], lst);

let rec typePathToFlowName = typePath =>
  switch (typePath) {
  | Path.Pident(id) => Ident.name(id)
  | Pdot(p, s, _pos) => typePathToFlowName(p) ++ s
  | Papply(p1, p2) =>
    typePathToFlowName(p1)
    ++ "__unsupported_genFlow__"
    ++ typePathToFlowName(p2)
  };

let needsArgConversion = ((lbl, c)) =>
  lbl !== NamedArgs.Nolabel || c !== Identity;

let rec extract_fun = (revArgDeps, revArgs, typ) =>
  Types.(
    switch (typ.desc) {
    | Tlink(t) => extract_fun(revArgDeps, revArgs, t)
    | Tarrow("", t1, t2, _) =>
      let (deps, convertableFlowType) = reasonTypeToConversion(t1);
      let nextRevDeps = List.append(deps, revArgDeps);
      extract_fun(
        nextRevDeps,
        [(NamedArgs.Nolabel, convertableFlowType), ...revArgs],
        t2,
      );
    | Tarrow(lbl, t1, t2, _) =>
      switch (removeOption(lbl, t1)) {
      | None =>
        /* TODO: Convert name to object, convert null to optional. */
        let (deps, t1Conversion) = reasonTypeToConversion(t1);
        let nextRevDeps = List.rev_append(deps, revArgDeps);
        extract_fun(
          nextRevDeps,
          [(Label(lbl), t1Conversion), ...revArgs],
          t2,
        );
      | Some((lbl, t1)) =>
        let (deps, (t1Converter, t1FlowType)) = reasonTypeToConversion(t1);
        let t1Conversion = (OptionalArgument(t1Converter), t1FlowType);
        let nextRevDeps = List.append(deps, revArgDeps);
        /* TODO: Convert name to object, convert null to optional. */
        extract_fun(
          nextRevDeps,
          [(OptLabel(lbl), t1Conversion), ...revArgs],
          t2,
        );
      }
    | _ =>
      let (retDeps, (retConverter, retType)) = reasonTypeToConversion(typ);
      let (labeledConverters, labeledFlow) = distributeSplitRev(revArgs);
      /* TODO: Ignore all final single unit args at convert/type conversion time. */
      let notJustASingleUnitArg =
        switch (labeledConverters) {
        | [(Nolabel, c)] when c === Unit => false
        | _ => true
        };
      let needsArgConversion =
        List.exists(needsArgConversion, labeledConverters)
        && notJustASingleUnitArg;
      let allDeps = List.append(List.rev(revArgDeps), retDeps);
      let revGroupedFlow = NamedArgs.groupReversed([], [], labeledFlow);
      let groupedFlow = NamedArgs.reverse(revGroupedFlow);
      let flowArgs = itm =>
        switch (itm) {
        | NamedArgs.NamedArgs(rows) => Flow.ObjectType(rows)
        | Arg(flowType) => flowType
        };
      let flowArrow =
        Flow.Arrow([], List.map(flowArgs, groupedFlow), retType);
      let functionConverter =
        retConverter !== Identity || needsArgConversion ?
          Fn((labeledConverters, retConverter)) : Identity;
      (allDeps, (functionConverter, flowArrow));
    }
  )
/**
 * Convertes Types.type_expr to:
 *
 *   (list(dependency), option(expressionConverter), renderedFlowType)).
 *
 * - `list(dependency)`: The Reason types types used in the type structure.  It
 * must be ensured that these are then imported.
 * - `option(expressionConverter)`: Any converter that is required for this type.
 * - `renderedFlowType` the flow type (just a string prepresentation)
 * TODO: Handle the case where the function in Reason accepts a single unit
 * arg, which should NOT be converted.
 */
and reasonTypeToConversion = (typ: Types.type_expr): conversionPlan =>
  Types.(
    switch (typ.desc) {
    | Tvar(None) =>
      let typeName = jsTypeNameForAnonymousTypeID(typ.id);
      (
        [FreeTypeVariable(typeName, typ.id)],
        (Identity, Flow.Ident(typeName, [])),
      );
    | Tvar(Some(s)) =>
      let typeName = s;
      (
        [FreeTypeVariable(typeName, typ.id)],
        (Identity, Flow.Ident(s, [])),
      );
    | Tconstr(Pdot(Path.Pident({Ident.name: "FB"}), "bool", _), [], _)
    | Tconstr(Path.Pident({name: "bool"}), [], _) => (
        [],
        (Identity, Flow.Ident("bool", [])),
      )
    | Tconstr(Pdot(Path.Pident({Ident.name: "FB"}), "int", _), [], _)
    | Tconstr(Path.Pident({name: "int"}), [], _) => (
        [],
        (Identity, Flow.Ident("number", [])),
      )
    | Tconstr(Pdot(Path.Pident({Ident.name: "FB"}), "string", _), [], _)
    | Tconstr(Path.Pident({name: "string"}), [], _) => (
        [],
        (Identity, Flow.Ident("string", [])),
      )
    | Tconstr(Pdot(Path.Pident({Ident.name: "FB"}), "unit", _), [], _)
    | Tconstr(Path.Pident({name: "unit"}), [], _) => (
        [],
        (Unit, Flow.Ident("(typeof undefined)", [])),
      )
    /*
     * Arrays do not experience any conversion, in order to retain referencial
     * equality. This poses a problem for Arrays that contain option types
     * which require conversion. The solution here could be to use the Reason
     * representation of option for everything except named arguments.
     */
    | Tconstr(Pdot(Path.Pident({Ident.name: "FB"}), "array", _), [p], _)
    | Tconstr(Path.Pident({name: "array"}), [p], _) =>
      let (paramDeps, (itemConverter, itemFlow)) =
        reasonTypeToConversion(p);
      if (itemConverter === Identity) {
        (paramDeps, (Identity, Flow.Ident("$ReadOnlyArray", [itemFlow])));
      } else {
        raise(
          Invalid_argument(
            "Converting Arrays with elements that require conversion "
            ++ "is not yet supported. Saw an array containing type:"
            ++ Flow.render(itemFlow),
          ),
        );
      };
    | Tconstr(Pdot(Path.Pident({Ident.name: "FB"}), "option", _), [p], _)
    | Tconstr(Path.Pident({name: "option"}), [p], _) =>
      /* TODO: Handle / verify the case of nested optionals. */
      let (paramDeps, (paramConverter, paramConverted)) =
        reasonTypeToConversion(p);
      let composedConverter = Option(paramConverter);
      (paramDeps, (composedConverter, Flow.Optional(paramConverted)));
    | Tarrow(_) => extract_fun([], [], typ)
    | Tlink(t) => reasonTypeToConversion(t)
    | Tconstr(path, [], _) => (
        [TypeAtPath(path)],
        (Identity, Flow.Ident(typePathToFlowName(path), [])),
      )
    /* This type doesn't have any built in converter. But what if it was a
     * genFlow variant type? */
    /*
     * Built-in standard library parameterized types (aside from option) are
     * like custom parameterized types in that they don't undergo conversion,
     * and their type parameter's dependencies are tracked.  For example
     * `list(int)` will be treated just like a custom type named List that.
     * There is special treatment of TypeAtPath("list") to make sure the
     * built-in JS type defs are brought in from the right location.
     */
    | Tconstr(path, typeParams, _) =>
      let (typeParamDeps, convertableFlowType) =
        reasonTypeToConversionMany(typeParams);
      /* How is this exprConv completely ignored? */
      let typeArgs =
        List.map(
          ((exprConv, flowTyp: Flow.typ)) => flowTyp,
          convertableFlowType,
        );
      (
        [TypeAtPath(path), ...typeParamDeps],
        (Identity, Flow.Ident(typePathToFlowName(path), typeArgs)),
      );
    | _ => ([], (Identity, Flow.anyAlias))
    }
  )
and reasonTypeToConversionMany = args => {
  let (deps, convertableFlowTypes) =
    List.split(List.map(reasonTypeToConversion, args));
  (List.concat(deps), convertableFlowTypes);
};

let variantLeafTypeName = (typeName, leafName) =>
  String.capitalize(typeName) ++ String.capitalize(leafName);

module Dependencies = {
  /**
     * Allows checking if there exists a polymorphic dep before creating several
     * list coppies.
     */
  let rec hasTypeVar = deps =>
    switch (deps) {
    | [] => false
    | [FreeTypeVariable(_s, _), ...tl] => true
    | [_, ...tl] => hasTypeVar(tl)
    };
  /*
   * A little bit of n squared never hurt anyone for n < 5.
   */
  let extractFreeTypeVars = deps =>
    if (hasTypeVar(deps)) {
      List.fold_left(
        ((curFreeTypeVars, curNonFreeTypeVars) as soFar, next) =>
          switch (next) {
          | FreeTypeVariable(s, id) =>
            List.exists(((s2, id2)) => s2 == s, curFreeTypeVars) ?
              soFar : ([(s, id), ...curFreeTypeVars], curNonFreeTypeVars)
          | _ => (curFreeTypeVars, [next, ...curNonFreeTypeVars])
          },
        ([], []),
        deps,
      );
    } else {
      ([], deps);
    };
  let filterFreeTypeVars = (freeTypeVars, deps) =>
    List.filter(
      fun
      | FreeTypeVariable(s, id) =>
        !List.exists(((s2, id2)) => id == id2, freeTypeVars)
      | _ => true,
      deps,
    );
};

module TypeVars = {
  /**
   * Extracts type variables from dependencies.
   */
  let extractOne = (soFar, typ) =>
    switch (typ) {
    | {Types.id, desc: Tvar(None)} =>
      let typeName = jsTypeNameForAnonymousTypeID(id);
      [(typeName, id), ...soFar];
    | {id, desc: Tvar(Some(s))} =>
      let typeName = s;
      [(typeName, id), ...soFar];
    | _ => soFar
    };

  /*
   * Utility for extracting results of compiling to output.
   * Input:
   *
   *     [
   *       ([dep, dep], [itm, itm]),
   *       ([dep, dep], [itm, itm])
   *     ]
   *
   * Output:
   *
   * List.merge
   *     ([dep, dep, dep, dep], [itm, itm, itm, itm])
   */

  let extract = typeParams => {
    let typeVarnamesAndIDs = List.fold_left(extractOne, [], typeParams);
    List.rev(typeVarnamesAndIDs);
  };
  /*
   * A little bit of n squared never hurt anyone for n < 5.
   */
  let names = freeTypeVars => List.map(((name, id)) => name, freeTypeVars);
  let toFlow = freeTypeVars =>
    List.map(((name, id)) => Flow.Ident(name, []), freeTypeVars);
};

let createFunctionFlowType =
    (generics, argConvertableFlowTypes, resultFlowType) =>
  if (argConvertableFlowTypes === []) {
    resultFlowType;
  } else {
    let args = List.map(((_, flowTyp)) => flowTyp, argConvertableFlowTypes);
    Flow.Arrow(generics, args, resultFlowType);
  };

/*
 * TODO: Make the types namespaced by nested Flow module.
 */
let codeItemsFromConstructorDeclaration =
    (variantTypeName, constructorDeclaration, unboxedCounter, boxedCounter) => {
  let constructorArgs = constructorDeclaration.Types.cd_args;
  let leafName = Ident.name(constructorDeclaration.Types.cd_id);
  let (deps, convertableFlowTypes) =
    reasonTypeToConversionMany(constructorArgs);
  /* A valid Reason identifier that we can point UpperCase JS exports to. */
  let leafTypeName = variantLeafTypeName(variantTypeName, leafName);
  let (freeTypeVars, remainingDeps) = Dependencies.extractFreeTypeVars(deps);
  let flowTypeVars = TypeVars.toFlow(freeTypeVars);
  let retType = Flow.Ident(leafTypeName, flowTypeVars);
  let constructorFlowType =
    createFunctionFlowType(flowTypeVars, convertableFlowTypes, retType);
  let runTimeValue =
    if (constructorArgs == []) {
      let v = unboxedCounter^;
      incr(unboxedCounter);
      v;
    } else {
      let v = boxedCounter^;
      incr(boxedCounter);
      v;
    };
  let codeItems = [
    codeItemForType(~opaque=true, flowTypeVars, leafTypeName, Flow.anyAlias),
    ConstructorBinding(
      constructorFlowType,
      convertableFlowTypes,
      leafName,
      runTimeValue,
    ),
  ];
  (retType, (remainingDeps, codeItems));
};

let codeItemsForId = (~moduleName, ~valueBinding, id) => {
  let {Typedtree.vb_expr} = valueBinding;
  let expressionType = vb_expr.exp_type;
  let conversion = reasonTypeToConversion(expressionType);
  let (valueDeps, (converter, flowType)) = conversion;
  /*
   * We pull apart the polymorphic type variables at the binding level, but
   * not at deeper function types because we know that the Reason/OCaml type
   * system doesn't support higher ranked polymorphism, and so all type
   * variables most likely belong at the binding level.
   */
  let (freeTypeVars, remainingDeps) =
    Dependencies.extractFreeTypeVars(valueDeps);
  let flowTypeVars = TypeVars.toFlow(freeTypeVars);
  let flowType = Flow.abstractTheTypeParameters(flowType, flowTypeVars);
  let codeItems = [
    FlowTypeBinding(Ident.name(id), flowType),
    ValueBinding(moduleName, Ident.name(id), converter),
  ];
  (remainingDeps, codeItems);
};

/*
 * The `make` function is typically of the type:
 *
 *    (~named, ~args=?, 'childrenType) => ReasonReactComponentSpec<
 *      State,
 *      State,
 *      RetainedProps,
 *      RetainedProps,
 *      Action,
 *    >)
 *
 * We take a reference to that function and turn it into a React component of
 * type:
 *
 *
 *     exports.component = (component : React.Component<Props>);
 *
 * Where `Props` is of type:
 *
 *     {named: number, args?: number}
 */

let codeItemsForMake = (~moduleName, ~valueBinding, id) => {
  let {Typedtree.vb_expr} = valueBinding;
  let expressionType = vb_expr.exp_type;
  let conversion = reasonTypeToConversion(expressionType);
  let (valueDeps, (converter, flowType)) = conversion;
  let (freeTypeVars, remainingDeps) =
    Dependencies.extractFreeTypeVars(valueDeps);
  let flowTypeVars = TypeVars.toFlow(freeTypeVars);
  let flowType = Flow.abstractTheTypeParameters(flowType, flowTypeVars);
  switch (flowType) {
  | Flow.Arrow(
      _,
      [propOrChildren, ...childrenOrNil],
      Flow.Ident(
        "ReasonReactComponentSpec" | "ReactComponentSpec",
        [state, ..._],
      ),
    ) =>
    let flowPropGenerics =
      switch (childrenOrNil) {
      /* Then we only extracted a function that accepts children, no props */
      | [] => None
      /* Then we had both props and children. */
      | [_children, ..._] => Some(propOrChildren)
      };
    let propsTypeName = GenIdent.propsTypeName();
    let propsTypeNameFlow = Flow.Ident(propsTypeName, []);
    /* TODO: Polymorphic props */
    let componentFlowType =
      Flow.Ident(
        "React$ComponentType",
        switch (flowPropGenerics) {
        | None => []
        | Some(propsType) => [propsTypeNameFlow]
        },
      );
    let propsTypeDeclaration =
      switch (flowPropGenerics) {
      | None => []
      | Some(propsType) => [
          codeItemForType(~opaque=false, [], propsTypeName, propsType),
        ]
      };

    let items =
      propsTypeDeclaration
      @ [
        FlowTypeBinding("component", componentFlowType),
        ComponentBinding(
          moduleName,
          flowPropGenerics,
          id,
          converter,
          propsTypeName,
        ),
      ];
    let deps = [
      JSTypeFromModule("Component", "ReactComponent", "React"),
      ...remainingDeps,
    ];
    (deps, items);
  | _ =>
    /* not a component: treat make as a normal function */
    id |> codeItemsForId(~moduleName, ~valueBinding)
  };
};

/**
 * See how this binding is accessed/generated.
 * [@bs.module] external workModeConfig : Js.t('untypedThing) = "WorkModeConfig";
 * Js_unsafe.raw_expr("require('MyModule')");
 */
/**
 * This is where all the logic for mapping from a CMT file, to a parsetree Ast
 * which has injected Flow types and generated interop code.
 * Each @genFlow binding will have the following two:
 *
 *     let __flowTypeValueAnnotation__bindingName = "someFlowType";
 *     let bindingName =
 *       require('ModuleWhereOrigBindingLives.bs').bindingName;
 *
 * Where the "someFlowType" is a flow converted type from Reason type, and
 * where the require() redirection may perform some safe conversions.
 */
let fromValueBinding = (~moduleName, valueBinding) => {
  let {Typedtree.vb_pat, vb_expr, vb_attributes} = valueBinding;
  switch (vb_pat.pat_desc, getGenFlowKind(vb_attributes)) {
  | (Tpat_var(id, _), GenFlow) when Ident.name(id) == "make" =>
    id |> codeItemsForMake(~moduleName, ~valueBinding)
  | (Tpat_var(id, _), GenFlow) =>
    id |> codeItemsForId(~moduleName, ~valueBinding)
  | _ => ([], [])
  };
};

let hasSomeGADTLeaf = constructorDeclarations =>
  List.exists(
    declaration => declaration.Types.cd_res !== None,
    constructorDeclarations,
  );

let fromTypeDecl = (dec: Typedtree.type_declaration) =>
  switch (
    dec.typ_type.type_params,
    dec.typ_type.type_kind,
    getGenFlowKind(dec.typ_attributes),
  ) {
  | (typeParams, Type_record(_, _), GenFlow | GenFlowOpaque) =>
    let freeTypeVars = TypeVars.extract(typeParams);
    let flowTypeVars = TypeVars.toFlow(freeTypeVars);
    let typeName = Ident.name(dec.typ_id);
    (
      [],
      [codeItemForType(~opaque=true, flowTypeVars, typeName, Flow.anyAlias)],
    );
  /*
   * This case includes aliasings such as:
   *
   *     type list('t) = List.t('t');
   */
  | (typeParams, Type_abstract, GenFlow | GenFlowOpaque)
  | (typeParams, Type_variant(_), GenFlowOpaque) =>
    let freeTypeVars = TypeVars.extract(typeParams);
    let flowTypeVars = TypeVars.toFlow(freeTypeVars);
    let typeName = Ident.name(dec.typ_id);
    switch (dec.typ_manifest) {
    | None => (
        [],
        [
          codeItemForType(~opaque=true, flowTypeVars, typeName, Flow.anyAlias),
        ],
      )
    | Some(coreType) =>
      let (deps, (_converter, flowType)) =
        reasonTypeToConversion(coreType.Typedtree.ctyp_type);
      let structureItems = [
        codeItemForType(~opaque=true, flowTypeVars, typeName, flowType),
      ];
      let deps = Dependencies.filterFreeTypeVars(freeTypeVars, deps);
      (deps, structureItems);
    };
  | (typeParams, Type_variant(constructorDeclarations), GenFlow)
      when !hasSomeGADTLeaf(constructorDeclarations) =>
    let variantTypeName = Ident.name(dec.typ_id);
    let resultTypesDepsAndVariantLeafBindings = {
      let unboxedCounter = ref(0);
      let boxedCounter = ref(0);
      List.map(
        constructorDeclaration =>
          codeItemsFromConstructorDeclaration(
            variantTypeName,
            constructorDeclaration,
            unboxedCounter,
            boxedCounter,
          ),
        constructorDeclarations,
      );
    };
    let (resultTypes, depsAndVariantLeafBindings) =
      List.split(resultTypesDepsAndVariantLeafBindings);
    let (listListDeps, listListItems) =
      List.split(depsAndVariantLeafBindings);
    let deps = List.concat(listListDeps);
    let items = List.concat(listListItems);
    let flowTypeVars = TypeVars.toFlow(TypeVars.extract(typeParams));
    let unionType =
      ExportUnionType({
        typeParams: flowTypeVars,
        leafTypes: resultTypes,
        name: variantTypeName,
      });
    (deps, List.append(items, [unionType]));
  | _ => ([], [])
  };

let importToString = import =>
  switch (import) {
  | ImportComment(s) => s
  | ImportAsFrom(jsTypeName, jsTypeNameAs, jsModuleName) =>
    "import type {"
    ++ jsTypeName
    ++ " as "
    ++ jsTypeNameAs
    ++ "} from '"
    ++ jsModuleName
    ++ "'"
  };

/**
 * Returns the path to import a given Reason module name.
 */
let importPathForReasonModuleName =
    (~outputFileRelative, ~resolver, ~modulesMap, ~moduleName) => {
  let tentative = ModuleName.toString(moduleName) ++ ".bs";
  StringMap.mem(tentative, modulesMap) ?
    StringMap.find(tentative, modulesMap) :
    ModuleResolver.resolveGeneratedModule(
      ~outputFileRelative,
      ~resolver,
      moduleName,
    );
};

let typePathToImport = (~outputFileRelative, ~resolver, ~modulesMap, typePath) =>
  switch (typePath) {
  | Path.Pident(id) when Ident.name(id) == "list" =>
    ImportAsFrom("List", "List", "ReasonPervasives.bs")

  | Path.Pident(id) =>
    ImportComment(
      "// No need to import locally visible type "
      ++ Ident.name(id)
      ++ ". Make sure it is also marked with @genFlow",
    )

  | Pdot(Papply(_, _), _, _)
  | Papply(_, _) => ImportComment("// Cannot import type with Papply")

  | Pdot(p, s, _pos) =>
    let moduleName =
      switch (p) {
      | Path.Pident(id) => id |> Ident.name |> ModuleName.fromString
      | Pdot(_, lastNameInPath, _) => lastNameInPath |> ModuleName.fromString
      | Papply(_, _) => assert(false) /* impossible: handled above */
      };
    ImportAsFrom(
      s,
      typePathToFlowName(typePath),
      importPathForReasonModuleName(
        ~outputFileRelative,
        ~resolver,
        ~modulesMap,
        ~moduleName,
      ),
    );
  };

let rec typePathsEqual = (a, b) =>
  switch (a, b) {
  | (Path.Pident(idA), Path.Pident(idB)) =>
    Ident.name(idA) == Ident.name(idB)
  | (Pdot(pA, sA, _), Pdot(pB, sB, _)) =>
    sA == sB && typePathsEqual(pA, pB)
  | (Path.Papply(_), Path.Papply(_))
  | (_, _) => false
  };

let dependencyEqual = (a, b) =>
  switch (a, b) {
  | (TypeAtPath(pA), TypeAtPath(pB)) => typePathsEqual(pA, pB)
  | (JSTypeFromModule(sA, sB, sC), JSTypeFromModule(sD, sE, sF)) =>
    sA == sD && sB == sE && sC == sF
  | (FreeTypeVariable(sA, idA), FreeTypeVariable(sB, idB)) =>
    sA == sB && idA === idB
  | _ => false
  };

let importCompare = (i1, i2) =>
  switch (i1, i2) {
  | (ImportComment(s1), ImportComment(s2)) => compare(s1, s2)
  | (ImportComment(_), _) => (-1)
  | (_, ImportComment(_)) => 1
  | (ImportAsFrom(x1, y1, z1), ImportAsFrom(x2, y2, z2)) =>
    let x = compare(x1, x2);
    x != 0 ?
      x :
      {
        let y = compare(y1, y2);
        y != 0 ? y : compare(z1, z2);
      };
  };

let fromDependencies =
    (~outputFileRelative, ~resolver, ~modulesMap, dependencies): list(t) => {
  let dependencyToImport = dependency =>
    switch (dependency) {
    | TypeAtPath(p) =>
      typePathToImport(~outputFileRelative, ~resolver, ~modulesMap, p)
    | JSTypeFromModule(typeName, asName, jsModuleName) =>
      ImportAsFrom(typeName, asName, jsModuleName)
    | FreeTypeVariable(s, id) =>
      ImportComment("// Warning polymorphic type unhandled:" ++ s)
    /* TODO: Currently unused. Would be useful for injecting dependencies
     * on runtime converters that end up being used. */
    };
  dependencies
  |> List.map(dependencyToImport)
  |> List.sort_uniq(importCompare)
  |> List.map(import => ImportType(import));
};