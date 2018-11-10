open GenTypeCommon;

let flowExpectedError = "// $FlowExpectedError: Reason checked type sufficiently\n";

let fileHeader = (~config) =>
  switch (config.language) {
  | Flow =>
    let strictness = "strict";
    "/** \n * @flow "
    ++ strictness
    ++ "\n * @"
    ++ "generated \n * @nolint\n */\n";
  | TypeScript => "/* TypeScript file generated by genType. */\n"
  | Untyped => "/* Untyped file generated by genType. */\n"
  };

let generatedFilesExtension = (~config) =>
  switch (config.generatedFileExtension) {
  | Some(s) => s
  | None => ".gen"
  };

let outputFileSuffix = (~config) =>
  switch (config.language) {
  | Flow
  | Untyped => generatedFilesExtension(~config) ++ ".js"
  | TypeScript => generatedFilesExtension(~config) ++ ".tsx"
  };

let generatedModuleExtension = (~config) => generatedFilesExtension(~config);

let shimExtension = (~config) =>
  switch (config.language) {
  | Flow => ".shim.js"
  | TypeScript => ".shim.ts"
  | Untyped => ".shim.not.used"
  };

let genericsString = (~typeVars) =>
  typeVars === [] ? "" : "<" ++ String.concat(",", typeVars) ++ ">";

module Indent = {
  let break = (~indent) =>
    switch (indent) {
    | None => ""
    | Some(s) => "\n" ++ s
    };

  let more = (~indent) =>
    switch (indent) {
    | None => None
    | Some(s) => Some("  " ++ s)
    };

  let heuristic = (~indent, fields) =>
    fields |> List.length > 2 && indent == None ? Some("") : indent;
};

let interfaceName = (~config, name) =>
  config.exportInterfaces ? "I" ++ name : name;

let rec renderTyp =
        (~config, ~indent=None, ~typeNameIsInterface, ~inFunType, typ) =>
  switch (typ) {
  | Array(typ, arrayKind) =>
    let typIsSimple =
      switch (typ) {
      | Ident(_)
      | TypeVar(_) => true
      | _ => false
      };

    if (config.language == TypeScript && typIsSimple && arrayKind == Mutable) {
      (typ |> renderTyp(~config, ~indent, ~typeNameIsInterface, ~inFunType))
      ++ "[]";
    } else {
      let arrayName =
        arrayKind == Mutable ?
          "Array" :
          config.language == Flow ? "$ReadOnlyArray" : "ReadonlyArray";
      arrayName
      ++ "<"
      ++ (
        typ |> renderTyp(~config, ~indent, ~typeNameIsInterface, ~inFunType)
      )
      ++ ">";
    };

  | Enum({cases, _}) =>
    cases
    |> List.map(case => case.labelJS |> EmitText.quotes)
    |> String.concat(" | ")

  | Function({typeVars, argTypes, retType}) =>
    renderFunType(
      ~config,
      ~indent,
      ~typeNameIsInterface,
      ~inFunType,
      ~typeVars,
      argTypes,
      retType,
    )

  | GroupOfLabeledArgs(fields)
  | Object(fields)
  | Record(fields) =>
    let indent1 = fields |> Indent.heuristic(~indent);
    fields
    |> renderFields(
         ~config,
         ~indent=indent1,
         ~typeNameIsInterface,
         ~inFunType,
       );

  | Ident(identPath, typeArguments) =>
    (
      config.exportInterfaces && identPath |> typeNameIsInterface ?
        identPath |> interfaceName(~config) : identPath
    )
    ++ genericsString(
         ~typeVars=
           typeArguments
           |> List.map(
                renderTyp(~config, ~indent, ~typeNameIsInterface, ~inFunType),
              ),
       )
  | Nullable(typ)
  | Option(typ) =>
    switch (config.language) {
    | Flow
    | Untyped =>
      "?"
      ++ (
        typ |> renderTyp(~config, ~indent, ~typeNameIsInterface, ~inFunType)
      )
    | TypeScript =>
      "(null | undefined | "
      ++ (
        typ |> renderTyp(~config, ~indent, ~typeNameIsInterface, ~inFunType)
      )
      ++ ")"
    }
  | Tuple(innerTypes) =>
    "["
    ++ (
      innerTypes
      |> List.map(
           renderTyp(~config, ~indent, ~typeNameIsInterface, ~inFunType),
         )
      |> String.concat(", ")
    )
    ++ "]"
  | TypeVar(s) => s
  }
and renderField =
    (
      ~config,
      ~indent,
      ~typeNameIsInterface,
      ~inFunType,
      {name: lbl, optional, mutable_, typ},
    ) => {
  let optMarker = optional === Optional ? "?" : "";
  let mutMarker =
    mutable_ == Immutable ? config.language == Flow ? "+" : "readonly " : "";
  Indent.break(~indent)
  ++ mutMarker
  ++ lbl
  ++ optMarker
  ++ ": "
  ++ (typ |> renderTyp(~config, ~indent, ~typeNameIsInterface, ~inFunType));
}
and renderFields =
    (~config, ~indent, ~typeNameIsInterface, ~inFunType, fields) => {
  let indent1 = Indent.more(~indent);
  (
    config.language == Flow && !config.exportInterfaces && fields != [] ?
      "{|" : "{"
  )
  ++ String.concat(
       ", ",
       List.map(
         renderField(
           ~config,
           ~indent=indent1,
           ~typeNameIsInterface,
           ~inFunType,
         ),
         fields,
       ),
     )
  ++ Indent.break(~indent)
  ++ (
    config.language == Flow && !config.exportInterfaces && fields != [] ?
      "|}" : "}"
  );
}
and renderFunType =
    (
      ~config,
      ~indent,
      ~typeNameIsInterface,
      ~inFunType,
      ~typeVars,
      argTypes,
      retType,
    ) =>
  (inFunType ? "(" : "")
  ++ genericsString(~typeVars)
  ++ "("
  ++ String.concat(
       ", ",
       List.mapi(
         (i, t) => {
           let parameterName =
             config.language == Flow ?
               "" : "_" ++ string_of_int(i + 1) ++ ":";
           parameterName
           ++ (
             t
             |> renderTyp(
                  ~config,
                  ~indent,
                  ~typeNameIsInterface,
                  ~inFunType=true,
                )
           );
         },
         argTypes,
       ),
     )
  ++ ") => "
  ++ (
    retType |> renderTyp(~config, ~indent, ~typeNameIsInterface, ~inFunType)
  )
  ++ (inFunType ? ")" : "");

let typToString = (~config, ~typeNameIsInterface, typ) =>
  typ |> renderTyp(~config, ~typeNameIsInterface, ~inFunType=false);

let ofType = (~config, ~typeNameIsInterface, ~typ, s) =>
  config.language == Untyped ?
    s : s ++ ": " ++ (typ |> typToString(~config, ~typeNameIsInterface));

let emitExportConst_ =
    (
      ~early,
      ~comment="",
      ~emitters,
      ~name,
      ~typeNameIsInterface,
      ~typ,
      ~config,
      line,
    ) =>
  (comment == "" ? comment : "// " ++ comment ++ "\n")
  ++ (
    switch (config.module_, config.language) {
    | (_, TypeScript)
    | (ES6, _) =>
      "export const "
      ++ (name |> ofType(~config, ~typeNameIsInterface, ~typ))
      ++ " = "
      ++ line
    | (CommonJS, _) =>
      "const "
      ++ (name |> ofType(~config, ~typeNameIsInterface, ~typ))
      ++ " = "
      ++ line
      ++ ";\nexports."
      ++ name
      ++ " = "
      ++ name
    }
  )
  |> (early ? Emitters.exportEarly : Emitters.export)(~emitters);

let emitExportConst = emitExportConst_(~early=false);

let emitExportConstEarly = emitExportConst_(~early=true);

let emitExportConstMany =
    (~emitters, ~name, ~typeNameIsInterface, ~typ, ~config, lines) =>
  lines
  |> String.concat("\n")
  |> emitExportConst(~emitters, ~name, ~typeNameIsInterface, ~typ, ~config);

let emitExportFunction =
    (~early, ~comment="", ~emitters, ~name, ~config, line) =>
  (comment == "" ? comment : "// " ++ comment ++ "\n")
  ++ (
    switch (config.module_, config.language) {
    | (_, TypeScript)
    | (ES6, _) => "export function " ++ name ++ line
    | (CommonJS, _) =>
      "function " ++ name ++ line ++ ";\nexports." ++ name ++ " = " ++ name
    }
  )
  |> (early ? Emitters.exportEarly : Emitters.export)(~emitters);

let emitExportDefault = (~emitters, ~config, name) =>
  switch (config.module_, config.language) {
  | (_, TypeScript)
  | (ES6, _) =>
    "export default " ++ name ++ ";" |> Emitters.export(~emitters)
  | (CommonJS, _) =>
    "exports.default = " ++ name ++ ";" |> Emitters.export(~emitters)
  };

let emitExportType =
    (
      ~early=false,
      ~emitters,
      ~config,
      ~opaque,
      ~typeVars,
      ~optTyp,
      ~typeNameIsInterface,
      resolvedTypeName,
    ) => {
  let export = early ? Emitters.exportEarly : Emitters.export;
  let typeParamsString = genericsString(~typeVars);
  let isInterface = resolvedTypeName |> typeNameIsInterface;
  let resolvedTypeName =
    config.exportInterfaces && isInterface ?
      resolvedTypeName |> interfaceName(~config) : resolvedTypeName;

  switch (config.language) {
  | Flow =>
    switch (optTyp) {
    | Some(typ) when config.exportInterfaces && isInterface && !opaque =>
      "export interface "
      ++ resolvedTypeName
      ++ typeParamsString
      ++ " "
      ++ (
        (opaque ? mixedOrUnknown(~config) : typ)
        |> typToString(~config, ~typeNameIsInterface)
      )
      ++ ";"
      |> export(~emitters)
    | Some(typ) =>
      "export"
      ++ (opaque ? " opaque " : " ")
      ++ "type "
      ++ resolvedTypeName
      ++ typeParamsString
      ++ " = "
      ++ (
        (opaque ? mixedOrUnknown(~config) : typ)
        |> typToString(~config, ~typeNameIsInterface)
      )
      ++ ";"
      |> export(~emitters)
    | None =>
      "export"
      ++ (opaque ? " opaque " : " ")
      ++ "type "
      ++ (resolvedTypeName |> EmitText.brackets)
      ++ ";"
      |> export(~emitters)
    }
  | TypeScript =>
    if (opaque) {
      /* Represent an opaque type as an absract class with a field called 'opaque'.
         Any type parameters must occur in the type of opaque, so that different
         instantiations are considered different types. */
      let typeOfOpaqueField =
        typeVars == [] ? "any" : typeVars |> String.concat(" | ");
      "// tslint:disable-next-line:max-classes-per-file \n"
      ++ (
        String.capitalize(resolvedTypeName) != resolvedTypeName ?
          "// tslint:disable-next-line:class-name\n" : ""
      )
      ++ "export abstract class "
      ++ resolvedTypeName
      ++ typeParamsString
      ++ " { protected opaque!: "
      ++ typeOfOpaqueField
      ++ " }; /* simulate opaque types */"
      |> export(~emitters);
    } else {
      (
        if (isInterface && config.exportInterfaces) {
          "export interface " ++ resolvedTypeName ++ typeParamsString ++ " ";
        } else {
          "// tslint:disable-next-line:interface-over-type-literal\n"
          ++ "export type "
          ++ resolvedTypeName
          ++ typeParamsString
          ++ " = ";
        }
      )
      ++ (
        switch (optTyp) {
        | Some(typ) => typ |> typToString(~config, ~typeNameIsInterface)
        | None => resolvedTypeName
        }
      )
      ++ ";"
      |> export(~emitters);
    }
  | Untyped => emitters
  };
};

let emitExportVariantType =
    (
      ~emitters,
      ~config,
      ~name,
      ~typeParams,
      ~typeNameIsInterface,
      ~variants: list(variant),
    ) =>
  switch (config.language) {
  | Flow
  | TypeScript =>
    "export type "
    ++ name
    ++ genericsString(
         ~typeVars=
           typeParams |> List.map(typToString(~config, ~typeNameIsInterface)),
       )
    ++ " =\n  | "
    ++ (
      variants
      |> List.map(({name, params}) =>
           Ident(name, params) |> typToString(~config, ~typeNameIsInterface)
         )
      |> String.concat("\n  | ")
    )
    ++ ";"
    |> Emitters.export(~emitters)
  | Untyped => emitters
  };

let emitImportValueAsEarly = (~config, ~emitters, ~name, ~nameAs, importPath) => {
  let commentBeforeImport =
    config.language == Flow ?
      "// flowlint-next-line nonstrict-import:off\n" : "";
  commentBeforeImport
  ++ "import "
  ++ (
    switch (nameAs) {
    | Some(nameAs) => "{" ++ name ++ " as " ++ nameAs ++ "}"
    | None => name
    }
  )
  ++ " from"
  ++ "'"
  ++ (importPath |> ImportPath.toString)
  ++ "';"
  |> Emitters.requireEarly(~emitters);
};

let emitRequire =
    (
      ~importedValueOrComponent,
      ~early,
      ~emitters,
      ~config,
      ~moduleName,
      ~strict,
      importPath,
    ) => {
  let commentBeforeRequire =
    switch (config.language) {
    | TypeScript => "// tslint:disable-next-line:no-var-requires\n"
    | Flow =>
      strict ?
        early ? "// flowlint-next-line nonstrict-import:off\n" : "" :
        flowExpectedError
    | Untyped => ""
    };
  switch (config.module_) {
  | ES6 when !importedValueOrComponent && config.language != TypeScript =>
    commentBeforeRequire
    ++ "import * as "
    ++ ModuleName.toString(moduleName)
    ++ " from '"
    ++ (importPath |> ImportPath.toString)
    ++ "';"
    |> (early ? Emitters.requireEarly : Emitters.require)(~emitters)
  | _ =>
    commentBeforeRequire
    ++ "const "
    ++ ModuleName.toString(moduleName)
    ++ " = require('"
    ++ (importPath |> ImportPath.toString)
    ++ "');"
    |> (early ? Emitters.requireEarly : Emitters.require)(~emitters)
  };
};

let require = (~early) => early ? Emitters.requireEarly : Emitters.require;

let emitRequireReact = (~early, ~emitters, ~config) =>
  switch (config.language) {
  | Flow =>
    emitRequire(
      ~importedValueOrComponent=false,
      ~early,
      ~emitters,
      ~config,
      ~moduleName=ModuleName.react,
      ~strict=false,
      ImportPath.react,
    )
  | TypeScript =>
    "import * as React from 'react';" |> require(~early, ~emitters)
  | Untyped => emitters
  };

let reactComponentType = (~config, ~propsTypeName) =>
  Ident(
    config.language == Flow ? "React$ComponentType" : "React.ComponentClass",
    [Ident(propsTypeName, [])],
  );

let componentExportName = (~config, ~fileName, ~moduleName) =>
  switch (config.language) {
  | Flow =>
    fileName == moduleName ? "component" : moduleName |> ModuleName.toString
  | _ => moduleName |> ModuleName.toString
  };

let emitImportTypeAs =
    (
      ~emitters,
      ~config,
      ~typeName,
      ~asTypeName,
      ~typeNameIsInterface,
      ~importPath,
    ) => {
  let (typeName, asTypeName) =
    switch (asTypeName) {
    | Some(asName) =>
      asName |> typeNameIsInterface ?
        (
          typeName |> interfaceName(~config),
          Some(asName |> interfaceName(~config)),
        ) :
        (typeName, asTypeName)
    | None => (typeName, asTypeName)
    };
  let strictLocalPrefix =
    config.language == Flow ?
      "// flowlint-next-line nonstrict-import:off\n" : "";
  switch (config.language) {
  | Flow
  | TypeScript =>
    strictLocalPrefix
    ++ "import "
    ++ (config.language == Flow ? "type " : "")
    ++ "{"
    ++ typeName
    ++ (
      switch (asTypeName) {
      | Some(asT) => " as " ++ asT
      | None => ""
      }
    )
    ++ "} from '"
    ++ (importPath |> ImportPath.toString)
    ++ "';"
    |> Emitters.import(~emitters)
  | Untyped => emitters
  };
};

let ofTypeAny = (~config, s) =>
  config.language == TypeScript ? s ++ ": any" : s;

let emitTypeCast = (~config, ~typ, ~typeNameIsInterface, s) =>
  switch (config.language) {
  | TypeScript =>
    s ++ " as " ++ (typ |> typToString(~config, ~typeNameIsInterface))
  | Untyped
  | Flow => s
  };