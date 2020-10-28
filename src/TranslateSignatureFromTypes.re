open GenTypeCommon;

/* Like translateTypeDeclaration but from Types not Typedtree */
let translateTypeDeclarationFromTypes =
    (
      ~config,
      ~outputFileRelative,
      ~resolver,
      ~typeEnv,
      ~id,
      {type_attributes, type_kind, type_loc, type_manifest, type_params}: Types.type_declaration,
    )
    : list(CodeItem.typeDeclaration) => {
  typeEnv |> TypeEnv.newType(~name=id |> Ident.name);
  let typeName = Ident.name(id);
  let typeVars = type_params |> TypeVars.extractFromTypeExpr;
  if (Debug.translation^) {
    Log_.item("Translate Types.type_declaration %s\n", typeName);
  };

  let declarationKind =
    switch (type_kind) {
    | Type_record(labelDeclarations, _) =>
      TranslateTypeDeclarations.RecordDeclarationFromTypes(labelDeclarations)

    | Type_variant(constructorDeclarations)
        when
          !TranslateTypeDeclarations.hasSomeGADTLeaf(constructorDeclarations) =>
      VariantDeclarationFromTypes(constructorDeclarations)

    | Type_abstract => GeneralDeclarationFromTypes(type_manifest)

    | _ => NoDeclaration
    };

  declarationKind
  |> TranslateTypeDeclarations.traslateDeclarationKind(
       ~config,
       ~loc=type_loc,
       ~outputFileRelative,
       ~resolver,
       ~typeAttributes=type_attributes,
       ~typeEnv,
       ~typeName,
       ~typeVars,
     );
};

/* Like translateModuleDeclaration but from Types not Typedtree */
let rec translateModuleDeclarationFromTypes =
        (
          ~config,
          ~outputFileRelative,
          ~resolver,
          ~typeEnv,
          ~id,
          moduleDeclaration: Types.module_declaration,
        )
        : Translation.t =>
  switch (moduleDeclaration.md_type) {
  | Mty_signature(signature) =>
    let name = id |> Ident.name;
    signature
    |> translateSignatureFromTypes(
         ~config,
         ~outputFileRelative,
         ~resolver,
         ~typeEnv=typeEnv |> TypeEnv.newModule(~name),
       )
    |> Translation.combine;

  | Mty_ident(_) =>
    logNotImplemented("Mty_ident " ++ __LOC__);
    Translation.empty;
  | Mty_functor(_) =>
    logNotImplemented("Mty_functor " ++ __LOC__);
    Translation.empty;
  | Mty_alias(_) =>
    logNotImplemented("Mty_alias " ++ __LOC__);
    Translation.empty;
  }
/* Like translateSignatureItem but from Types not Typedtree */
and translateSignatureItemFromTypes =
    (
      ~config,
      ~outputFileRelative,
      ~resolver,
      ~moduleItemGen,
      ~typeEnv,
      signatureItem: Types.signature_item,
    )
    : Translation.t =>
  switch (signatureItem) {
  | Types.Sig_type(id, typeDeclaration, _) => {
      importTypes: [],
      codeItems: [],
      typeDeclarations:
        typeDeclaration
        |> translateTypeDeclarationFromTypes(
             ~config,
             ~outputFileRelative,
             ~resolver,
             ~typeEnv,
             ~id,
           ),
    }

  | Types.Sig_module(id, moduleDeclaration, _) =>
    let moduleItem =
      moduleItemGen |> Runtime.newModuleItem(~name=id |> Ident.name);
    typeEnv |> TypeEnv.updateModuleItem(~moduleItem);
    moduleDeclaration
    |> translateModuleDeclarationFromTypes(
         ~config,
         ~outputFileRelative,
         ~resolver,
         ~typeEnv,
         ~id,
       );

  | Types.Sig_value(id, {val_attributes, val_loc, val_type}) =>
    if (val_attributes |> Annotation.fromAttributes(~loc=val_loc) == GenType) {
      id
      |> Ident.name
      |> Translation.translateValue(
           ~attributes=val_attributes,
           ~config,
           ~docString=Annotation.getDocString(val_attributes),
           ~outputFileRelative,
           ~resolver,
           ~typeEnv,
           ~typeExpr=val_type,
           ~addAnnotationsToFunction=t =>
           t
         );
    } else {
      Translation.empty;
    }
  | Types.Sig_typext(_) =>
    logNotImplemented("Sig_typext " ++ __LOC__);
    Translation.empty;
  | Types.Sig_modtype(_) =>
    logNotImplemented("Sig_modtype " ++ __LOC__);
    Translation.empty;
  | Types.Sig_class(_) =>
    logNotImplemented("Sig_class " ++ __LOC__);
    Translation.empty;
  | Types.Sig_class_type(_) =>
    logNotImplemented("Sig_class_type " ++ __LOC__);
    Translation.empty;
  }
/* Like translateSignature but from Types not Typedtree */
and translateSignatureFromTypes =
    (
      ~config,
      ~outputFileRelative,
      ~resolver,
      ~typeEnv,
      signature: list(Types.signature_item),
    )
    : list(Translation.t) => {
  if (Debug.translation^) {
    Log_.item("Translate Types.signature\n");
  };
  let moduleItemGen = Runtime.moduleItemGen();
  signature
  |> List.map(
       translateSignatureItemFromTypes(
         ~config,
         ~outputFileRelative,
         ~resolver,
         ~moduleItemGen,
         ~typeEnv,
       ),
     );
};
