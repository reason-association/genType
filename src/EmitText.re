type nameGen = Hashtbl.t(string, int);

let name = (~nameGen, s) =>
  switch (Hashtbl.find(nameGen, s)) {
  | n =>
    Hashtbl.replace(nameGen, s, n + 1);
    s ++ string_of_int(n + 1);
  | exception Not_found =>
    Hashtbl.replace(nameGen, s, 0);
    s;
  };

let parens = xs => "(" ++ (xs |> String.concat(", ")) ++ ")";

let arg = (~nameGen, x) => "Arg" ++ x |> name(~nameGen);

let argi = (~nameGen, i) => "Arg" ++ (i |> string_of_int) |> name(~nameGen);

let array = xs => "[" ++ (xs |> String.concat(", ")) ++ "]";

let brackets = x => "{ " ++ x ++ " }";

let comment = x => "/* " ++ x ++ " */";

let funCall = (~args, name) =>
  name ++ "(" ++ (args |> String.concat(", ")) ++ ")";

let funDef = (~args, ~mkBody, functionName) => {
  let (params, vals) = List.split(args);
  "function "
  ++ (functionName == "" ? "_" : functionName)
  ++ (params |> parens)
  ++ " "
  ++ (vals |> mkBody |> brackets);
};

let newNameGen = () => Hashtbl.create(1);

let quotes = x => "\"" ++ x ++ "\"";

let resultName = (~nameGen) => "result" |> name(~nameGen);

let switch_ = (~cases, expr) => {
  let lastCase = (cases |> List.length) - 1;
  [
    cases
    |> List.mapi((i, (label, code)) =>
         if (i == lastCase) {
           code;
         } else {
           expr ++ "===" ++ label ++ " ? " ++ code ++ " : ";
         }
       )
    |> String.concat(" "),
  ]
  |> parens;
};

let typeOfObject = x => "typeof(" ++ x ++ ")" ++ " === " ++ "\'object\'";