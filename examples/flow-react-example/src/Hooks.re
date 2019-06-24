type vehicle = {name: string};

[@react.component]
let make = (~vehicle) => {
  let (count, setCount) = React.useState(() => 0);

  <div>
    <p>
      {React.string(
         "Hooks example "
         ++ vehicle.name
         ++ " clicked "
         ++ string_of_int(count)
         ++ " times",
       )}
    </p>
    <button onClick={_ => setCount(_ => count + 1)}>
      {React.string("Click me")}
    </button>
    <ImportHooks person={name: "Mary", age: 71} renderMe={_ => React.null}>
      {React.string("child1")}
      {React.string("child2")}
    </ImportHooks>
    <ImportHookDefault
      person={name: "DefaultImport", age: 42} renderMe={_ => React.null}>
      {React.string("child1")}
      {React.string("child2")}
    </ImportHookDefault>
  </div>;
};

[@genType]
let default = make;

[@genType]
[@react.component]
let anotherComponent = (~vehicle, ~callback: unit => unit) => {
  callback();
  <div> {React.string("Another Hook " ++ vehicle.name)} </div>;
};

module Inner = {
  [@genType]
  [@react.component]
  let make = (~vehicle) =>
    <div> {React.string("Another Hook " ++ vehicle.name)} </div>;

  [@genType]
  [@react.component]
  let anotherComponent = (~vehicle) =>
    <div> {React.string("Another Hook " ++ vehicle.name)} </div>;

  module Inner2 = {
    [@genType]
    [@react.component]
    let make = (~vehicle) =>
      <div> {React.string("Another Hook " ++ vehicle.name)} </div>;

    [@genType]
    [@react.component]
    let anotherComponent = (~vehicle) =>
      <div> {React.string("Another Hook " ++ vehicle.name)} </div>;
  };
};

module NoProps = {
  [@genType]
  [@react.component]
  let make = () => {
    <div> ReasonReact.null </div>;
  };
};

type cb = (~_to: vehicle) => unit;

[@genType]
let functionWithRenamedArgs = (~_to, ~_Type, ~cb: cb) => {
  cb(~_to);
  _to.name ++ _Type.name;
};

[@genType]
[@react.component]
let componentWithRenamedArgs = (~_to, ~_Type, ~cb: cb) => {
  cb(~_to);
  React.string(_to.name ++ _Type.name);
};

[@genType]
[@react.component]
let makeWithRef = (~vehicle, ref) => {
  switch (ref->Js.Nullable.toOption) {
  | Some(ref) =>
    <button ref={ReactDOMRe.Ref.domRef(ref)}>
      {React.string(vehicle.name)}
    </button>
  | None => React.null
  };
};

[@genType]
let testForwardRef = React.forwardRef(makeWithRef);

[@genType]
type callback('input, 'output) = React.callback('input, 'output);

[@genType]
type testReactContext = React.Context.t(int);

[@genType]
type testReactRef = React.Ref.t(int);

[@genType]
[@react.component]
let polymorphicComponent = (~p as (x, _)) => React.string(x.name);

[@genType]
[@react.component]
let functionReturningReactElement = (~name) => React.string(name);