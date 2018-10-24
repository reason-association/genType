open GenTypeCommon;

type declarationKind =
  | RecordDelaration(list(Types.label_declaration))
  | GeneralDeclaration(option(Typedtree.core_type))
  | VariantDeclaration(list(Types.constructor_declaration))
  | ImportTypeDeclaration(string, option(Annotation.attributePayload))
  | NoDeclaration;

let createExportType =
    (~opaque, ~typeVars, ~optTyp, ~annotation, ~typeEnv, typeName)
    : CodeItem.exportFromTypeDeclaration => {
  let resolvedTypeName = typeName |> TypeEnv.addModulePath(~typeEnv);
  {
    exportKind: ExportType({opaque, typeVars, resolvedTypeName, optTyp}),
    annotation,
  };
};

let variantLeafTypeName = (typeName, leafName) =>
  String.capitalize(typeName) ++ String.capitalize(leafName);

let translateConstructorDeclaration =
    (
      ~config as {language, _} as config,
      ~outputFileRelative,
      ~resolver,
      ~recordGen,
      ~annotation,
      ~typeEnv,
      variantTypeName,
      constructorDeclaration,
    ) => {
  let constructorArgs = constructorDeclaration.Types.cd_args;
  let leafName = constructorDeclaration.Types.cd_id |> Ident.name;
  let leafNameResolved = leafName |> TypeEnv.addModulePath(~typeEnv);
  let argsTranslation =
    Dependencies.translateTypeExprs(~language, ~typeEnv, constructorArgs);
  let argTypes = argsTranslation |> List.map(({Dependencies.typ, _}) => typ);
  let importTypes =
    argsTranslation
    |> List.map(({Dependencies.dependencies, _}) => dependencies)
    |> List.concat
    |> List.map(
         Translation.pathToImportType(
           ~config,
           ~outputFileRelative,
           ~resolver,
         ),
       )
    |> List.concat;
  /* A valid Reason identifier that we can point UpperCase JS exports to. */

  let variantTypeNameResolved =
    variantLeafTypeName(variantTypeName, leafName)
    |> TypeEnv.addModulePath(~typeEnv);

  let typeVars = argTypes |> TypeVars.freeOfList;

  let variant = {
    name: variantTypeNameResolved,
    params: typeVars |> TypeVars.toTyp,
  };
  let constructorTyp: CodeItem.constructorTyp = {typeVars, argTypes, variant};
  let recordValue =
    recordGen |> Runtime.newRecordValue(~unboxed=constructorArgs == []);
  let exportFromTypeDeclaration = {
    CodeItem.exportKind:
      ExportVariantLeaf({
        exportType: {
          opaque: Some(true),
          typeVars,
          resolvedTypeName: variantTypeNameResolved,
          optTyp: Some(mixedOrUnknown(~language)),
        },
        constructorTyp,
        argTypes,
        leafName: leafNameResolved,
        recordValue,
      }),
    annotation,
  };

  (variant, {Translation.importTypes, exportFromTypeDeclaration});
};

let traslateDeclarationKind =
    (
      ~config as {language, _} as config,
      ~outputFileRelative,
      ~resolver,
      ~typeEnv,
      ~annotation,
      ~typeName,
      ~typeVars,
      ~typeParams,
      declarationKind,
    )
    : list(Translation.typeDeclaration) =>
  switch (declarationKind) {
  | GeneralDeclaration(optCoreType) =>
    switch (optCoreType) {
    | None => [
        {
          importTypes: [],
          exportFromTypeDeclaration:
            typeName
            |> createExportType(
                 ~opaque=Some(true),
                 ~typeVars,
                 ~optTyp=Some(mixedOrUnknown(~language)),
                 ~annotation,
                 ~typeEnv,
               ),
        },
      ]
    | Some(coreType) =>
      let typeExprTranslation =
        coreType.Typedtree.ctyp_type
        |> Dependencies.translateTypeExpr(~language, ~typeEnv);
      let opaque = annotation == GenTypeOpaque ? Some(true) : None /* None means don't know */;
      let typ =
        switch (optCoreType, typeExprTranslation.typ) {
        | (Some({ctyp_desc: Ttyp_variant(rowFields, _, _), _}), Enum(enum))
            when rowFields |> List.length == (enum.cases |> List.length) =>
          let cases =
            List.combine(rowFields, enum.cases)
            |> List.map(((field, case)) =>
                 switch (field) {
                 | Typedtree.Ttag(label, attributes, _, _) =>
                   switch (
                     attributes
                     |> Annotation.getAttributePayload(
                          Annotation.tagIsGenTypeAs,
                        )
                   ) {
                   | Some(StringPayload(asLabel)) => {
                       label,
                       labelJS: asLabel,
                     }
                   | _ => {label, labelJS: label}
                   }
                 | Tinherit(_) => case
                 }
               );
          cases |> createEnum;
        | _ => typeExprTranslation.typ
        };
      let exportFromTypeDeclaration =
        typeName
        |> createExportType(
             ~opaque,
             ~typeVars,
             ~optTyp=Some(typ),
             ~annotation,
             ~typeEnv,
           );

      [
        {
          importTypes:
            typeExprTranslation.dependencies
            |> Translation.translateDependencies(
                 ~config,
                 ~outputFileRelative,
                 ~resolver,
               ),
          exportFromTypeDeclaration,
        },
      ];
    }

  | RecordDelaration(labelDeclarations) =>
    let fieldTranslations =
      labelDeclarations
      |> List.map(({Types.ld_id, ld_mutable, ld_type, ld_attributes, _}) => {
           let name =
             switch (
               ld_attributes
               |> Annotation.getAttributePayload(Annotation.tagIsGenTypeAs)
             ) {
             | Some(StringPayload(s)) => s
             | _ => ld_id |> Ident.name
             };
           let mutability = ld_mutable == Mutable ? Mutable : Immutable;
           (
             name,
             mutability,
             ld_type |> Dependencies.translateTypeExpr(~language, ~typeEnv),
           );
         });
    let importTypes =
      fieldTranslations
      |> List.map(((_, _, {Dependencies.dependencies, _})) => dependencies)
      |> List.concat
      |> List.map(
           Translation.pathToImportType(
             ~config,
             ~outputFileRelative,
             ~resolver,
           ),
         )
      |> List.concat;
    let fields =
      fieldTranslations
      |> List.map(((name, mutable_, {Dependencies.typ, _})) => {
           let (optional, typ1) =
             switch (typ) {
             | Option(typ1) => (Optional, typ1)
             | _ => (Mandatory, typ)
             };
           {name, optional, mutable_, typ: typ1};
         });
    let optTyp = Some(Record(fields));
    let typeVars = TypeVars.extract(typeParams);
    let opaque = Some(annotation == GenTypeOpaque);
    [
      {
        importTypes,
        exportFromTypeDeclaration:
          typeName
          |> createExportType(
               ~opaque,
               ~typeVars,
               ~optTyp,
               ~annotation,
               ~typeEnv,
             ),
      },
    ];

  | VariantDeclaration(constructorDeclarations) =>
    let leafTypesDepsAndVariantLeafBindings = {
      let recordGen = Runtime.recordGen();
      constructorDeclarations
      |> List.map(constructorDeclaration =>
           translateConstructorDeclaration(
             ~config,
             ~outputFileRelative,
             ~resolver,
             ~recordGen,
             ~annotation,
             ~typeEnv,
             typeName,
             constructorDeclaration,
           )
         );
    };
    let (variants, declarations) =
      leafTypesDepsAndVariantLeafBindings |> List.split;
    let typeParams = TypeVars.(typeParams |> extract |> toTyp);
    let variantTypeNameResolved = typeName |> TypeEnv.addModulePath(~typeEnv);
    let unionType = {
      CodeItem.exportKind:
        ExportVariantType({
          typeParams,
          variants,
          name: variantTypeNameResolved,
        }),
      annotation,
    };
    declarations @ [{exportFromTypeDeclaration: unionType, importTypes: []}];

  | ImportTypeDeclaration(importString, genTypeAsPayload) =>
    let typeName_ = typeName;
    let nameWithModulePath = typeName_ |> TypeEnv.addModulePath(~typeEnv);
    let (typeName, asTypeName) =
      switch (genTypeAsPayload) {
      | Some(StringPayload(asString)) => (
          asString,
          Some(nameWithModulePath),
        )
      | _ => (nameWithModulePath, None)
      };
    let importTypes = [
      {
        Translation.typeName,
        asTypeName,
        importPath: importString |> ImportPath.fromStringUnsafe,
        cmtFile: None,
      },
    ];
    let exportFromTypeDeclaration =
      /* Make the imported type usable from other modules by exporting it too. */
      typeName_
      |> createExportType(
           ~opaque=Some(false),
           ~typeVars=[],
           ~optTyp=None,
           ~annotation=Generated,
           ~typeEnv,
         );

    [{importTypes, exportFromTypeDeclaration}];

  | NoDeclaration => []
  };

let hasSomeGADTLeaf = constructorDeclarations =>
  List.exists(
    declaration => declaration.Types.cd_res !== None,
    constructorDeclarations,
  );

let translateTypeDeclaration =
    (
      ~config,
      ~outputFileRelative,
      ~resolver,
      ~typeEnv,
      ~annotation: Annotation.t,
      {typ_id, typ_type, typ_attributes, typ_manifest, _}: Typedtree.type_declaration,
    )
    : list(Translation.typeDeclaration) => {
  typeEnv |> TypeEnv.newType(~name=typ_id |> Ident.name);
  let typeName = Ident.name(typ_id);
  let typeParams = typ_type.type_params;
  let typeVars = TypeVars.extract(typeParams);

  let declarationKind =
    switch (typ_type.type_kind, annotation) {
    | (Type_record(labelDeclarations, _), _) =>
      RecordDelaration(labelDeclarations)

    | (Type_variant(constructorDeclarations), _)
        when !hasSomeGADTLeaf(constructorDeclarations) =>
      VariantDeclaration(constructorDeclarations)

    | (Type_abstract, _) =>
      switch (
        typ_attributes
        |> Annotation.getAttributePayload(Annotation.tagIsGenTypeImport)
      ) {
      | Some(StringPayload(importString)) when typeParams == [] =>
        ImportTypeDeclaration(
          importString,
          typ_attributes
          |> Annotation.getAttributePayload(Annotation.tagIsGenTypeAs),
        )

      | _ => GeneralDeclaration(typ_manifest)
      }

    | _ => NoDeclaration
    };

  declarationKind
  |> traslateDeclarationKind(
       ~config,
       ~outputFileRelative,
       ~resolver,
       ~typeEnv,
       ~annotation,
       ~typeName,
       ~typeVars,
       ~typeParams,
     );
};

let translateTypeDeclarations =
    (
      ~config,
      ~outputFileRelative,
      ~resolver,
      ~typeEnv,
      typeDeclarations: list(Typedtree.type_declaration),
    )
    : list(Translation.typeDeclaration) => {
  let annotation =
    switch (typeDeclarations) {
    | [dec, ..._] => dec.typ_attributes |> Annotation.fromAttributes
    | [] => NoGenType
    };
  typeDeclarations
  |> List.map(
       translateTypeDeclaration(
         ~config,
         ~outputFileRelative,
         ~resolver,
         ~typeEnv,
         ~annotation,
       ),
     )
  |> List.concat;
};