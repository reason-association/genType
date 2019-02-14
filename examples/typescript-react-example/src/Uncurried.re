[@genType]
let uncurried0 = (.) => "";

[@genType]
let uncurried1 = (. x) => x |> string_of_int;

[@genType]
let uncurried2 = (. x, y) => (x |> string_of_int) ++ y;

[@genType]
let uncurried3 =
  (. x, y, z) => (x |> string_of_int) ++ y ++ (z |> string_of_int);

[@genType]
let curried3 = (x, y, z) => (x |> string_of_int) ++ y ++ (z |> string_of_int);

[@genType]
let callback = cb => cb() |> string_of_int;

type auth = {login: unit => string};

[@genType]
let callback2 = auth => auth.login();