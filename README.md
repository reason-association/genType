# Reason genType

[![CircleCI](https://circleci.com/gh/cristianoc/genType/tree/master.svg?style=svg)](https://circleci.com/gh/cristianoc/genType/tree/master) [![Appveyor](https://ci.appveyor.com/api/projects/status/github/cristianoc/gentype?svg=true)](https://ci.appveyor.com/project/cristianoc/gentype)

`genType` lets you export [Reason](https://reasonml.github.io/) values and types to use in JavaScript, and import JavaScript values and types into Reason, idiomatically. Converter functions between the two representations are generated based on the type of the value. The converters can be generated in vanilla JavaScript, or in [TypeScript](https://www.typescriptlang.org/) / [Flow](https://flow.org/en/) for a type-safe idiomatic interface.
In particular, conversion of [ReasonReact](https://reasonml.github.io/reason-react/) components both ways is supported, with automatic generation of the wrappers.

Here's an article describing how to use `genType` as part of a migration strategy where a tree of components is gradually converted to Reason bottom-up: [Adopting Reason: strategies, dual sources of truth, and why genType is a big deal](https://medium.com/p/c514265b466d).

The implementation of [@genType] performs a type-directed transformation of Reason programs after [bucklescript](https://github.com/BuckleScript/bucklescript) compilation. The transformed programs operate on data types idiomatic to JS. For example, a Reason function operating on a Reason record `{x:3}` (which is represented as `[3]` at runtime) is exported to a JS function operating on the corresponding JS object `{x:3}`.

The output of `genType` can be configured by using one of 3 back-ends: `untyped` to generate wrappers in vanilla JS, `typescript` to generate [TypeScript](https://www.typescriptlang.org/), and `flow` to generate JS with [Flow](https://flow.org/en/) type annotations.

[Here is a video illustrating the conversion of a ReasonReact component.](https://youtu.be/EV12EbxCPjM)
[![IMAGE ALT TEXT HERE](assets/genTypeDemo.png)](https://youtu.be/EV12EbxCPjM)

# Project status.

See [Changes.md](Changes.md) for a complete list of features, fixes, and changes for each release.

## Breaking Change:
> From version 2.0.0, ordinary variants (e.g. ``` | A | B(int) ```) use the same representation as polymorphic variants (e.g. ``` | `a | `b(int) ```). The construcor functions (e.g. ```A``` or ```B(42) ```) are not generated anymore. Instead, one can construct the variant values directly in JS. Check out the [variants](#variants) section below for more details.

> **Disclaimer:** While most of the feature set is complete, the project is still growing and changing based on feedback. It is possible that the workflow will change in future.

# Requirements

bs-platform 4.0.5 or higher

# Installation

Install the binaries via `npm`:

```
npm install --save-dev gentype

# Test running gentype
npx gentype --help
```

Add a `gentypeconfig` section to your `bsconfig.json` (See [Configuration](#configuration) for details):

```
"gentypeconfig": {
    "language": "untyped",
    "shims": {},
    "debug": {
      "all": false,
      "basic": false
    }
}
```

For running `gentype` with BuckleScript via `npm` workflow, add following script in your `package.json`:

```
scripts: {
  "bs:build": "export BS_CMT_POST_PROCESS_CMD=\"gentype\" && bsb -make-world",
  "bs:clean": "bsb -clean-world"
}
```

For running `gentype` via different mechanics (global env variable etc.), you can set `BS_CMT_POST_PROCESS_CMD` to `node_modules/.bin/gentype` as well.

With this configuration, BuckleScript will call `gentype` for each newly built file. You might want to clean your build artifacts before usage: `npx bsb -clean-world` (otherwise there might be cached values and no `.re.js` files are generated).

Check out the [Examples](#examples) for detailed setups (TypeScript, Flow and Plain JavaScript).

## Adding shims (TypeScript & Flow)

Configure your shim files in your `"gentypeconfig"` in [`bsconfig.json`](examples/typescript-react-example/bsconfig.json), and add relevant `.shims.js` files in a directory which is visible by bucklescript e.g. [`src/shims/`](examples/typescript-react-example/src/shims). An example shim to export ReactEvent can be found [here](examples/typescript-react-example/src/shims/ReactEvent.shim.ts).

## Testing the whole setup

Open any relevant `*.re` file and add `[@genType]` annotations to any bindings / values / functions to be used from JavaScript. If an annotated value uses a type, the type must be annotated too. See e.g. [ReasonComponent.re](examples/typescript-react-example/src/ReasonComponent.re).

Save the file and rebuild the project with BuckleScript. You should now see a `*.gen.tsx` (for TypeScript, or `*.gen.js` for Flow) file with the same name (e.g. `MyComponent.re` -> `MyComponent.gen.tsx`).

# Examples

We prepared some examples to give you an idea on how to integrate `genType` in your own project. Check out the READMEs of the listed projects.

**Please make sure to build genType before trying to build the examples.**

- [flow-react-example](examples/flow-react-example/README.md)
- [typescript-react-example](examples/typescript-react-example/README.md)
- [untyped-react-example](examples/untyped-react-example/README.md)

# Documentation

`genType` operates on two kinds of entities: _types_ and _values_.
Each can be _exported_ from Reason to JS, or _imported_ into Reason from JS.
The main annotation is `@genType`, which by default means _export_.

### Export and Import Types

The following exports a function type `callback` to JS:

```reason
[@genType]
type callback = ReactEvent.Mouse.t => unit;
```

To instead import a type called `complexNumber` from JS module `MyMath.ts` (or `MyMath.js`), use the `@genType.import` annotation:

```reason
[@genType.import "./MyMath"]
type complexNumber;
```

This imported type will be treated as opaque by Reason.

### Export and Import Values

To export a function `callback` to JS:

```reason
[@genType]
let callback = _ => Js.log("Clicked");
```

To import a function `realValue` from JS module `MyMath.ts` (or `MyMath.js`):

```reason
[@genType.import "./MyMath"] /* This is the module to import from. */
[@bs.module "./WrapJsValue.gen"] /* Always the name of the current file plus ".gen". */
/* Name and type of the JS value to import. */
external realValue: complexNumber => float = "";

```

Because of the `external` keyword, it's clear from context that this is an import, so you can also just use `@genType` and omit `.import`.

To import a default JS export, use `@genType.as "default"`. Similarly, to import a value with a different JS name, use e.g. `@genType.as "ValueStartingWithUpperCaseLetter"`. To import nested values, e.g. `Some.Nested.value`, use `@genType.as "Some.Nested.value"`. 

**NOTE** The argument of `@bs.module`must always be the name of the current file plus `.gen` (In future, this could be automatically generated).
If the imported value is consumed directly from a module defined in another directory, the behaviour of bucklescript's `bs.module` annotation [can be surprising](https://github.com/cristianoc/genType/issues/106).

### Export and Import React Components

To export a ReasonReact component to JS, and automatically generate a wrapper for it, simply annotate the `make` function:

```reason
[@genType]
let make = (~onClick: callback, _children) => {
  ...component,
  render: _ => <div onClick> "Click me"->ReasonReact.string </div>,
};
```

**NOTE** the value `component` must also be defined, above `make` in the same module (also in the case of components defined in nested modules).


To import and wrap a ReactJS component for use by ReasonReact, the type of the `make` function is the only information required:

```reason
[@genType.import "./MyBanner"] /* Module with the JS component to be wrapped. */
[@bs.module "./ImportMyBanner.gen"] /* Always the name of the current file plus ".gen". */
/* The make function will be automatically generated from the types below. */
external make:
  (~show: bool, ~message: option(message)=?, 'a) =>
  ReasonReact.component(
    ReasonReact.stateless,
    ReasonReact.noRetainedProps,
    ReasonReact.actionless,
  ) =
  "";
```

The type of `make` must have a named argument for each prop in the JS component. Optional props have option type. The `make` function will be generated by `genType`.

This assumes that the JS component was exported with a default export. In case of named export, use `@genType.as "componentName"`. To import a nested component, use e.g. `@genType.as "Some.Nested.component"`. 

**NOTE** The argument of `@bs.module`must always be the name of the current file plus `.gen` (In future, this could be automatically generated).

### Type Expansion and @genType.opaque

If an exported type `persons` references other types in its definition, those types are also exported by default, as long as they are defined in the same file:

```reason
type name = string;
type surname = string;
type person = {name:name, surname:surname};

[@genType]
type persons = array(person);
```

If however you wish to hide from JS the fact that `name` and `surname` are strings, you can do it with the `@genType.opaque` annotation:

```reason
[@genType.opaque]
type name = string;
[@genType.opaque]
type surname = string;

type person = {
  name,
  surname,
};

[@genType]
type persons = array(person);
```

### Renaming and @genType.as

By default, entities with a given name are exported/imported with the same name. However, you might wish to change the appearence of the name on the JS side.
For example, in the case of a Reason keyword, such as `type`:

```reason
[@genType]
type shipment = {
  date: float,
  [@genType.as "type"]
  type_: string,
};
```

Or in the case of components:

```reason
[@genType]
let make =
  (~date: float) => [@genType.as "type"] (~type_: string) => ...
```

**NOTE** For technical reasons, it is not possible to rename the first argument of a function (it will be fixed once bucklescript supports OCaml 4.0.6).

You will also need to reach out for renaming when importing a capitalized type from JS, since types in Reason cannot be capitalized:

```reason
[@genType.import "./MyMath"]
[@genType.as "ComplexNumber"]
type complexNumber;
```

## Configuration

Every `genType` powered project requires a configuration item `"gentypeconfig"` at top level in the project's `bsconfig.json`. (The use of a separate file `gentypeconfig.json` is being deprecated). The option has following structure:

```ts
...
  "gentypeconfig": {
    "language": "typescript" | "flow" | "untyped",
    "shims": {
      "ReasonReact": "ReactShim"
    }
  }
```

- **language**

  - "typescript" : Generate `*.tsx` files written in TypeScript.
  - "flow": Generate `*.re.js` files with Flow type annotations.
  - "untyped": Generate `*.re.js` files in vanilla JavaScript.

- **shims**
  - e.g. `Array<string>` with format: `"ReasonModule=JavaScriptModule"`
  - Required to export certain basic Reason data types to JS when one cannot modify the sources to add annotations (e.g. exporting Reason lists)

## Types Supported

### int

Reason values e.g. `1`, `2`, `3` are unchanged. So they are exported to JS values of type `number`.

### float

Reason values e.g. `1.0`, `2.0`, `3.0` are unchanged. So they are exported to JS values of type `number`.

### string

Reason values e.g. `"a"`, `"b"`, `"c"` are unchanged. So they are exported to JS values of type `string`.

### optionals

Reason values of type e.g. `option(int)`, such as `None`, `Some(0)`, `Some(1)`, `Some(2)`, are exported to JS values `null`, `undefined`, `0`, `1`, `2`.
The JS values are unboxed, and `null`/`undefined` are conflated.
So the option type is exported to JS type `null` or `undefined` or `number`.

### nullables

Reason values of type e.g. `Js.Nullable.t(int)`, such as `Js.Nullable.null`, `Js.Nullable.undefined`, `Js.Nullable.return(0)`, `Js.Nullable.return(1)`, `Js.Nullable.return(2)`, are exported to JS values `null`, `undefined`, `0`, `1`, `2`.
The JS values are identical: there is no conversion unless the argument type needs conversion.

### records

Reason record values of type e.g. `{x:int}` such as `{x:0}`, `{x:1}`, `{x:2}`, are exported to JS object values `{x:0}`, `{x:1}`, `{x:2}`. This requires a change of runtime representation from arrays to objects.
So they are exported to JS values of type `{x:number}`.

Since records are immutable by default, their fields will be exported to readonly property types in Flow/TS. Mutable fields are specified in Reason by e.g. `{mutable mutableField: string}`.

The `@genType.as` annotation can be used to change the name of a field on the JS side of things. So e.g. `{[@genType.as "y"] x:int}` is exported as JS type `{y:int}`.

If one field of the Reason record has option type, this is exported to an optional JS field. So for example Reason type `{x: option(int)}` is exported as JS type `{x?: number}`.

### objects

Reason object values of type e.g. `{. "x":int}` such as `{"x": 0}`, `{"x": 1}`, `{"x": 2}`, are exported as identical JS object values `{x:0}`, `{x:1}`, `{x:2}`. This requires no conversion. So they are exported to JS values of type `{x:number}`.
A conversion is required only when the type of some field requires conversions.

Since objects are immutable by default, their fields will be exported to readonly property types in Flow/TS. Mutable fields are specified in Reason by e.g. `{. [@bs.set] "mutableField": string }`.

It is possible to mix object and option types, so for example the Reason type `{. "x":int, "y":option(string)}` exports to JS type `{x:number, ?y: string}`, requires no conversion, and allows option pattern matching on the Reason side.

### tuples

Reason tuple values of type e.g. `(int, string)` are exported as identical JS values of type `[number, string]`. This requires no conversion, unless one of types of the tuple items does.
While the type of Reason tuples is immutable, there's currently no mature enforcement in TS/Flow, so they're currenty exported to mutable tuples.

### variants


Ordinary variants (with capitalized cases, e.g. ``` | A | B(int) ```) and polymorphic variants (with a backtick, e.g. ``` | `A | `B(int) ```) are represented in the same way, so there's no difference from the point of view of JavaScript. Polymorphic variants don't have to be capitalized.

Variants can have an *unboxed*, or a *boxed* representation. The unboxed representation is used when there is at most one case with a payload, and that payload has object type; otherwise, a boxed representation is used. Object types are arrays, objects, records and tuples.

Variants without payloads are essentially sequences of identifiers.
E.g. type ``[@genType] type days = | Monday | Tuesday``.
The corresponding JS representation is `"Monday"`, `"Tuesday"`.
Similarly, polymorphic variant type ``[@genType] type days = [ | `Monday | `Tuesday ] `` has the same JS representation.


When at most one variant case has a payload, and if the payload is of object type, e.g.
``` [ | Unnamed | Named({. "name": string, "surname": string}) ] ```
then the representation is unboxed: JS values are e.g. `"Unnamed"` and
`{name: "hello", surname: "world"}`. Similarly for polymorphic variants.
Note that this unboxed representation does not use the label `"Named"` of the variant case with payload, because that value is distinguished from the other payload-less cases by its type: an object.

If there is more than one case with payload, or if the single payload has not type object, a boxed representation is used. The boxed representation has shape ```{tag: "someTag", value: someValue}```.
For example, type ```| A | B(int) | C(string)``` has values such as ```"A"``` and 
```{tag: "B", value: 42}``` and ```{tag: "C", value: "hello"}```.
Polymorhphic variants are treated similarly. Notice that payloads for polymorphic variants are always unary: ``` `Pair(int,int) ``` has a single payload of type `(int,int)`. Instead, ordinary variants distinguish between unary ``` Pair((int,int)) ``` and binary ``` Pair(int,int) ``` payloads. All those cases are represented in JS as ```{tag: "Pair", value: [3, 4]}```, and the conversion functions take care of the different Reason representations.

The `@genType.as` annotation can be used to modify the name emitted for a variant case on the JS side. So e.g. ``` | [@genType.as "Arenamed"] A``` exports Reason value `` A `` to JS value `"Arenamed"`.
Boolean/integer/float constants can be expressed as ``` | [@genType.as true] True ``` and ``` | [@genType.as 20] Twenty ``` and ``` | [@genType.as 0.5] Half ```. Similarly for polymorphic variants.
The `@genType.as` annotation can also be used on variants with payloads to determine what appears in `{ tag: ... }`.


For more examples, see [Enums.re](examples/typescript-react-example/src/Enums.re) and [EnumsWithPayload.re](examples/typescript-react-example/src/EnumsWithPayload.re).

**NOTE** When exporting/importing values that have polymorphic variant type, you have to use type annotations, and cannot rely on type inference. So instead of ```let monday = `Monday```, use ```let monday : days = `Monday```. The former does not work, as the type checker infers a tyoe without annotations.

### arrays

Arrays with elements of Reason type `t` are exported to JS arrays with elements of the corresponding JS type. If a conversion is required, a copy of the array is performed.

Immutable arrays are supported with the additional Reason library
[ImmutableArray.re/.rei](examples/typescript-react-example/src/ImmutableArray.rei), which currently needs to be added to your project.
The type `ImmutableArray.t(+'a)` is covariant, and is mapped to readonly array types in TS/Flow. As opposed to TS/Flow, `ImmutableArray.t` does not allow casting in either direction with normal arrays. Instead, a copy must be performed using `fromArray` and `toArray`.

### functions

Reason functions are exported as JS functions of the corresponding type.
So for example a Reason function `foo : int => int` is exported as a JS function from numbers to numbers.

If named arguments are present in the Reason type, they are grouped and exported as JS objects. For example `foo : (~x:int, ~y:int) => int` is exported as a JS function from objects of type `{x:number, y:number}` to numbers.

In case of mixed named and unnamed arguments, consecutive named arguments form separate groups. So e.g. `foo : (int, ~x:int, ~y:int, int, ~z:int) => int` is exported to a JS function of type `(number, {x:number, y:number}, number, {z:number}) => number`.

To specify how a named argument is exported to JS, use the `[@genType.as "name"]` annotation:

```reason
[@genType]
let make =
  (~date: float) => [@genType.as "type"] (~type_: string) => ...
```

**NOTE** For technical reasons, it is not possible to rename the first argument of a function (it will be fixed once bucklescript supports OCaml 4.0.6).

### components

ReasonReact components with props of Reason types `t1`, `t2`, `t3` are exported as reactjs components with props of the JS types corresponding to `t1`, `t2`, `t3`. The annotation is on the `make` function: `[@genType] let make ...`.

A file can export many components by defining them in sub-modules. The toplevel component is also exported as default.

### imported types

It's possible to import an existing TS/Flow type as an opaque type in Reason. For example,

```reason
[@genType.import "./SomeFlowTypes"] type weekday;
```

defines a type which maps to `weekday` in `SomeFlowTypes.js`.
See for example [Types.re](examples/flow-react-example/src/Types.re) and [SomeFlowTypes.js](examples/flow-react-example/src/SomeFlowTypes.js).

### recursive types

Recursive types which do not require a conversion are fully supported.
If a recursive type requires a conversion, only a shallow conversion is performed, and a warning comment is included in the output. (The alternative would be to perform an expensive conversion down a data structure of arbitrary size).
See for example [Types.re](examples/typescript-react-example/src/nested/Types.re).

### first class modules

Reason first class modules are converted from their array Reason runtime representation to JS Object types.
For example,

```reason
module type MT = { let x: int; let y: string; };
module M = { let y = "abc"; let x = 42; };
[@genType] let firstClassModule: module MT = (module M);
```

is exported as a JS object of type

```reason
{x: number, y: string}
```

Notice how the order of elements in the exported JS object is determined by the module type `MT` and not the module implementation `M`.

### polymorphic types

If a Reason type contains a type variable, the corresponding value is not converted. In other words, the conversion is the identity function. For example, a Reason function of type `{payload: 'a} => 'a` must treat the value of the payload as a black box, as a consequence of parametric polymorphism. If a typed back-end is used, the reason type is converted to the corresponding generic type.

#### exporting values from polymorphic types with hidden type variables

For cases when a value that contains a hidden type variable needs to be converted, a function can be used to produce the appropriate output:

**Doesn't work**

```reason
[@genType]
let none = None;
```

```js
export const none: ?T1 = OptionBS.none; // Errors out as T1 is not defined
```

**Works**

```reason
[@genType]
let none = () => None;
```

```js
const none = <T1>(a: T1): ?T1 => OptionBS.none;
```

# Experimental features

These features are for experimentation only. They could be changed/removed any time, and not be considered breaking changes.

- Export object and record types as interfaces. To activate, add `"exportInterfaces": true` to the configuration. The types are also renamed from `name` to `Iname`.

# Limitations

- **BuckleScript in-source = true**. Currently only supports bucklescript projects with [in-source generation](https://bucklescript.github.io/docs/en/build-configuration#package-specs) and `.bs.js` file suffix.

# Development/Contributing

Please check out our [development instructions](DEVELOPMENT.md).
