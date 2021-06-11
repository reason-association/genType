open GenTypeCommon

val emitExportConst :
  ?comment:string ->
  config:config ->
  ?docString:string ->
  emitters:Emitters.t ->
  name:string ->
  type_:type_ ->
  typeNameIsInterface:(string -> bool) ->
  string ->
  Emitters.t

val emitExportConstEarly :
  ?comment:string ->
  config:config ->
  ?docString:string ->
  emitters:Emitters.t ->
  name:string ->
  type_:type_ ->
  typeNameIsInterface:(string -> bool) ->
  string ->
  Emitters.t

val emitExportDefault : emitters:Emitters.t -> string -> Emitters.t

val emitExportType :
  ?early:bool ->
  config:config ->
  emitters:Emitters.t ->
  nameAs:string option ->
  opaque:bool ->
  type_:type_ ->
  typeNameIsInterface:(string -> bool) ->
  typeVars:string list ->
  string ->
  Emitters.t

val emitImportReact : emitters:Emitters.t -> Emitters.t

val emitImportTypeAs :
  emitters:Emitters.t ->
  config:config ->
  typeName:string ->
  asTypeName:string option ->
  typeNameIsInterface:(string -> bool) ->
  importPath:ImportPath.t ->
  Emitters.t

val emitImportValueAsEarly :
  config:config ->
  emitters:Emitters.t ->
  name:string ->
  nameAs:string option ->
  ImportPath.t ->
  Emitters.t

val emitRequire :
  importedValueOrComponent:bool ->
  early:bool ->
  emitters:Emitters.t ->
  config:config ->
  moduleName:ModuleName.t ->
  ImportPath.t ->
  Emitters.t

val emitTypeCast :
  config:config ->
  type_:type_ ->
  typeNameIsInterface:(string -> bool) ->
  string ->
  string

val fileHeader : config:config -> sourceFile:string -> string

val generatedModuleExtension : config:config -> string

val isTypeFunctionComponent : fields:fields -> type_ -> bool

val isTypeReactElement : type_ -> bool

val ofTypeAny : config:config -> string -> string
(** Help type-checking by making the argument of type any *)

val outputFileSuffix : config:config -> string

val shimExtension : string

val typeReactChild : type_

val typeReactContext : type_:type_ -> type_

val typeReactDOMReDomRef : type_

val typeReactElement : type_

val typeReactEventMouseT : type_

val typeReactRef : type_:type_ -> type_

val typeAny : type_

val typeToString :
  config:config -> typeNameIsInterface:(string -> bool) -> type_ -> string
