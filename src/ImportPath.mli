open GenTypeCommon

type t

val bsCurryPath : config:config -> t

val dump : t -> string

val emit : config:config -> t -> string

val fromModule : dir:string -> importExtension:string -> ModuleName.t -> t

val fromStringUnsafe : string -> t

val propTypes : t

val reasonReactPath : config:config -> t

val react : t

val toCmt : config:config -> outputFileRelative:string -> t -> string
