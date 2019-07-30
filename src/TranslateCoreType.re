open GenTypeCommon;

open! TranslateTypeExprFromTypes;

let removeOption = (~label, coreType: Typedtree.core_type) =>
  switch (coreType.ctyp_desc) {
  | Ttyp_constr(Path.Pident(id), _, [t])
      /* This has a different representation in 4.03+ */
      when Ident.name(id) == "option" && label != "" && label.[0] == '?' =>
    Some((String.sub(label, 1, String.length(label) - 1), t))
  | Ttyp_constr(Pdot(Path.Pident(nameSpace), id, _), _, [t])
      /* This has a different representation in 4.03+ */
      when
        Ident.name(nameSpace) == "FB"
        && id == "option"
        && label != ""
        && label.[0] == '?' =>
    Some((String.sub(label, 1, String.length(label) - 1), t))
  | _ => None
  };

type processVariant = {
  noPayloads: list((string, Typedtree.attributes)),
  payloads: list((string, Typedtree.attributes, Typedtree.core_type)),
  unknowns: list(string),
};

let processVariant = rowFields => {
  let rec loop = (~noPayloads, ~payloads, ~unknowns, fields) =>
    switch (fields) {
    | [
        Typedtree.Ttag(
          label,
          attributes,
          _,
          /* only variants with no payload */ [],
        ),
        ...otherFields,
      ] =>
      otherFields
      |> loop(
           ~noPayloads=[(label, attributes), ...noPayloads],
           ~payloads,
           ~unknowns,
         )
    | [Ttag(label, attributes, _, [payload]), ...otherFields] =>
      otherFields
      |> loop(
           ~noPayloads,
           ~payloads=[(label, attributes, payload), ...payloads],
           ~unknowns,
         )
    | [Ttag(_, _, _, [_, _, ..._]) | Tinherit(_), ...otherFields] =>
      otherFields
      |> loop(~noPayloads, ~payloads, ~unknowns=["Tinherit", ...unknowns])
    | [] => {
        noPayloads: noPayloads |> List.rev,
        payloads: payloads |> List.rev,
        unknowns: unknowns |> List.rev,
      }
    };
  rowFields |> loop(~noPayloads=[], ~payloads=[], ~unknowns=[]);
};

let rec translateArrowType =
        (
          ~config,
          ~typeVarsGen,
          ~noFunctionReturnDependencies=false,
          ~typeEnv,
          ~revArgDeps,
          ~revArgs,
          coreType: Typedtree.core_type,
        ) =>
  switch (coreType.ctyp_desc) {
  | Ttyp_arrow("", coreType1, coreType2) =>
    let {dependencies, type_} =
      coreType1 |> translateCoreType_(~config, ~typeVarsGen, ~typeEnv, _);
    let nextRevDeps = List.rev_append(dependencies, revArgDeps);
    coreType2
    |> translateArrowType(
         ~config,
         ~typeVarsGen,
         ~noFunctionReturnDependencies,
         ~typeEnv,
         ~revArgDeps=nextRevDeps,
         ~revArgs=[(Nolabel, type_), ...revArgs],
       );
  | Ttyp_arrow(label, coreType1, coreType2) =>
    let asLabel =
      switch (coreType.ctyp_attributes |> Annotation.getAttributeRenaming) {
      | Some(s) => s
      | None => ""
      };
    switch (coreType1 |> removeOption(~label)) {
    | None =>
      let {dependencies, type_: type1} =
        coreType1 |> translateCoreType_(~config, ~typeVarsGen, ~typeEnv);
      let nextRevDeps = List.rev_append(dependencies, revArgDeps);
      coreType2
      |> translateArrowType(
           ~config,
           ~typeVarsGen,
           ~noFunctionReturnDependencies,
           ~typeEnv,
           ~revArgDeps=nextRevDeps,
           ~revArgs=[
             (
               Label(
                 asLabel == "" ? label |> Runtime.mangleObjectField : asLabel,
               ),
               type1,
             ),
             ...revArgs,
           ],
         );
    | Some((lbl, t1)) =>
      let {dependencies, type_: type1} =
        t1 |> translateCoreType_(~config, ~typeVarsGen, ~typeEnv);
      let nextRevDeps = List.rev_append(dependencies, revArgDeps);
      coreType2
      |> translateArrowType(
           ~config,
           ~typeVarsGen,
           ~noFunctionReturnDependencies,
           ~typeEnv,
           ~revArgDeps=nextRevDeps,
           ~revArgs=[
             (
               OptLabel(
                 asLabel == "" ? lbl |> Runtime.mangleObjectField : asLabel,
               ),
               type1,
             ),
             ...revArgs,
           ],
         );
    };
  | _ =>
    let {dependencies, type_: retType} =
      coreType |> translateCoreType_(~config, ~typeVarsGen, ~typeEnv);
    let allDeps =
      List.rev_append(
        revArgDeps,
        noFunctionReturnDependencies ? [] : dependencies,
      );

    let labeledConvertableTypes = revArgs |> List.rev;
    let argTypes = labeledConvertableTypes |> NamedArgs.group;

    let functionType =
      Function({
        argTypes,
        componentName: None,
        retType,
        typeVars: [],
        uncurried: false,
      });

    {dependencies: allDeps, type_: functionType};
  }
and translateCoreType_ =
    (
      ~config,
      ~typeVarsGen,
      ~noFunctionReturnDependencies=false,
      ~typeEnv,
      coreType: Typedtree.core_type,
    ) =>
  switch (coreType.ctyp_desc) {
  | Ttyp_alias(ct, _) =>
    ct
    |> translateCoreType_(
         ~config,
         ~typeVarsGen,
         ~noFunctionReturnDependencies=false,
         ~typeEnv,
       )

  | Ttyp_constr(
      Pdot(Pident({name: "Js", _}), "t", _) as path,
      _,
      [
        {
          ctyp_desc:
            Ttyp_object(tObj, closedFlag) |
            Ttyp_alias({ctyp_desc: Ttyp_object(tObj, closedFlag)}, _),
          _,
        },
      ],
    ) =>
    let getFieldType = ((name, _attibutes, t)) => (
      name,
      name |> Runtime.isMutableObjectField ?
        {dependencies: [], type_: ident("")} :
        t |> translateCoreType_(~config, ~typeVarsGen, ~typeEnv),
    );
    let fieldsTranslations = tObj |> List.map(getFieldType);
    translateConstr(
      ~config,
      ~fieldsTranslations,
      ~closedFlag=closedFlag == Closed ? Closed : Open,
      ~paramsTranslation=[],
      ~path,
      ~typeEnv,
    );

  | Ttyp_constr(path, _, typeParams) =>
    let paramsTranslation =
      typeParams |> translateCoreTypes_(~config, ~typeVarsGen, ~typeEnv);
    TranslateTypeExprFromTypes.translateConstr(
      ~config,
      ~fieldsTranslations=[],
      ~closedFlag=Closed,
      ~paramsTranslation,
      ~path,
      ~typeEnv,
    );

  | Ttyp_poly(_, t) =>
    t
    |> translateCoreType_(
         ~config,
         ~typeVarsGen,
         ~noFunctionReturnDependencies,
         ~typeEnv,
       )

  | Ttyp_arrow(_) =>
    coreType
    |> translateArrowType(
         ~config,
         ~typeVarsGen,
         ~noFunctionReturnDependencies,
         ~typeEnv,
         ~revArgDeps=[],
         ~revArgs=[],
       )

  | Ttyp_tuple(listExp) =>
    let innerTypesTranslation =
      listExp |> translateCoreTypes_(~config, ~typeVarsGen, ~typeEnv);
    let innerTypes =
      innerTypesTranslation |> List.map(({type_, _}) => type_);
    let innerTypesDeps =
      innerTypesTranslation
      |> List.map(({dependencies, _}) => dependencies)
      |> List.concat;

    let tupleType = Tuple(innerTypes);

    {dependencies: innerTypesDeps, type_: tupleType};

  | Ttyp_var(s) => {dependencies: [], type_: TypeVar(s)}

  | Ttyp_variant(rowFields, _, _) =>
    switch (rowFields |> processVariant) {
    | {noPayloads, payloads, unknowns: []} =>
      let noPayloads =
        noPayloads
        |> List.map(((label, _attibutes)) =>
             {label, labelJS: StringLabel(label)}
           );
      let payloadsTranslations =
        payloads
        |> List.map(((label, attributes, payload)) =>
             (
               label,
               attributes,
               payload |> translateCoreType_(~config, ~typeVarsGen, ~typeEnv),
             )
           );
      let payloads =
        payloadsTranslations
        |> List.map(((label, _attributes, translation)) => {
             let numArgs = 1;
             (
               {label, labelJS: StringLabel(label)},
               numArgs,
               translation.type_,
             );
           });
      let type_ = createVariant(~noPayloads, ~payloads, ~polymorphic=true);
      let dependencies =
        payloadsTranslations
        |> List.map(((_, _, {dependencies, _})) => dependencies)
        |> List.concat;
      {dependencies, type_};

    | _ => {dependencies: [], type_: mixedOrUnknown(~config)}
    }

  | Ttyp_package({pack_path, pack_fields}) =>
    switch (typeEnv |> TypeEnv.lookupModuleTypeSignature(~path=pack_path)) {
    | Some((signature, typeEnv)) =>
      let typeEquationsTranslation =
        pack_fields
        |> List.map(((x, t)) =>
             (
               x.Asttypes.txt,
               t |> translateCoreType_(~config, ~typeVarsGen, ~typeEnv),
             )
           );
      let typeEquations =
        typeEquationsTranslation
        |> List.map(((x, translation)) => (x, translation.type_));
      let dependenciesFromTypeEquations =
        typeEquationsTranslation
        |> List.map(((_, translation)) => translation.dependencies)
        |> List.flatten;
      let typeEnv1 = typeEnv |> TypeEnv.addTypeEquations(~typeEquations);
      let (dependenciesFromRecordType, type_) =
        signature.sig_type
        |> signatureToRecordType(~config, ~typeVarsGen, ~typeEnv=typeEnv1);
      {
        dependencies:
          dependenciesFromTypeEquations @ dependenciesFromRecordType,
        type_,
      };
    | None => {dependencies: [], type_: mixedOrUnknown(~config)}
    }

  | Ttyp_any
  | Ttyp_class(_)
  | Ttyp_object(_) => {dependencies: [], type_: mixedOrUnknown(~config)}
  }
and translateCoreTypes_ =
    (~config, ~typeVarsGen, ~typeEnv, typeExprs): list(translation) =>
  typeExprs |> List.map(translateCoreType_(~config, ~typeVarsGen, ~typeEnv));

let translateCoreType =
    (~config, ~noFunctionReturnDependencies=?, ~typeEnv, coreType) => {
  let typeVarsGen = GenIdent.createTypeVarsGen();
  let translation =
    coreType
    |> translateCoreType_(
         ~config,
         ~typeVarsGen,
         ~noFunctionReturnDependencies?,
         ~typeEnv,
       );

  if (Debug.dependencies^) {
    translation.dependencies
    |> List.iter(dep => logItem("Dependency: %s\n", dep |> depToString));
  };
  translation;
};