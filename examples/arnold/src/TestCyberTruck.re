// A progress function will eventually terminate
let progress = {
  let counter = ref(Random.int(100));
  () => {
    if (counter^ < 0) {
      assert(false);
    };
    counter := counter^ - 1;
  };
};

// Another progress function
let progress2 = progress;

// A progress function can be taken from a module
module Progress = {
  module Nested = {
    let f = progress;
  };
};

// Need to declare at least one progress function and one recursive definition
[@progress progress]
let rec justReturn = () => ();

[@progress progress]
let rec alwaysLoop = () => alwaysLoop(); // infinite loop

[@progress progress]
let rec alwaysProgress = () => {
  // Terminates
  progress();
  alwaysProgress();
};

[@progress progress]
let rec alwaysProgressWrongOrder = () => {
  alwaysProgressWrongOrder();
  progress(); // Oops: this is too late
};

[@progress progress]
let rec doNotAlias = () => {
  // Must not alias recursive functions
  let alias = doNotAlias;
  alias();
};

[@progress (progress, progress2)]
// Terminates as each branch makes progress
let rec progressOnBothBranches = x => {
  if (x > 3) {
    progress();
  } else {
    progress2();
  };
  progressOnBothBranches(x);
};

[@progress progress]
// Loops as progress is only on one branch
let rec progressOnOneBranch = x => {
  if (x > 3) {
    progress();
  };
  progressOnOneBranch(x);
};

[@progress progress]
// callParseFunction is parametric: it takes a parse function and calls it
let rec testParametricFunction = x => {
  if (x > 3) {
    progress();
  };
  testParametricFunction2(x);
}
and testParametricFunction2 = x => {
  callParseFunction(x, ~parseFunction=testParametricFunction);
}
and callParseFunction = (x, ~parseFunction) => parseFunction(x); // loops

[@progress Progress.Nested.f]
let rec testCacheHit = x => {
  if (x > 0) {
    doNothing(x);
    doNothing(x); // this should hit the analysis cache
    Progress.Nested.f();
  };
  testCacheHit(x);
}
and doNothing = _ => ();

[@progress progress]
// Loops as can't rely on a specific evaluation order
let rec evalOrderIsNotLeftToRight = x => {
  let combineTwoUnits = ((), ()) => ();
  combineTwoUnits(progress(), evalOrderIsNotLeftToRight(x));
};

[@progress progress]
// Loops as can't rely on a specific evaluation order
let rec evalOrderIsNotRightToLeft = x => {
  let combineTwoUnits = ((), ()) => ();
  combineTwoUnits(evalOrderIsNotRightToLeft(x), progress());
};

[@progress progress]
// Terminates: all arguments are evaluated in some order
let rec butFirstArgumentIsAlwaysEvaluated = x => {
  let combineTwoUnits = ((), ()) => ();
  combineTwoUnits(progress(), ());
  butFirstArgumentIsAlwaysEvaluated(x);
};

[@progress progress]
// Terminates: all arguments are evaluated in some order
let rec butSecondArgumentIsAlwaysEvaluated = x => {
  let combineTwoUnits = ((), ()) => ();
  combineTwoUnits((), progress());
  butSecondArgumentIsAlwaysEvaluated(x);
};

module Parser = {
  type token =
    | Asterisk
    | Eof
    | Lparen
    | Int(int)
    | Plus
    | Rparen;

  type position = {
    lnum: int,
    cnum: int,
  };

  type t = {
    mutable position,
    mutable errors: list(string),
    mutable token,
  };

  let tokenToString = token =>
    switch (token) {
    | Asterisk => "*"
    | Eof => "Eof"
    | Lparen => "("
    | Int(n) => string_of_int(n)
    | Plus => "+"
    | Rparen => ")"
    };

  let next = p => {
    p.token = Random.bool() ? Eof : Int(Random.int(1000));
    p.position = {lnum: Random.int(1000), cnum: Random.int(80)};
  };

  let err = (p, s) => p.errors = [s, ...p.errors];

  let expect = (p, token) =>
    if (p.token == token) {
      next(p);
    } else {
      err(p, "expected token " ++ tokenToString(p.token));
    };
};

module Expr = {
  type t =
    | Int(int)
    | Plus(t, t);
};

let parseList = (p: Parser.t, ~f) => {
  let rec loop = (p: Parser.t) =>
    if (p.token == Asterisk) {
      [];
    } else {
      let item = f(p);
      let l = loop(p);
      [item, ...l];
    };
  loop(p);
};

let parseInt = (p: Parser.t) => {
  let res =
    switch (p.token) {
    | Int(n) => n
    | _ =>
      Parser.err(p, "integer expected");
      (-1);
    };
  Parser.next(p);
  res;
};

[@progress Parser.next]
let rec parseListInt = p => parseList(p, ~f=parseInt)

[@progress]
and parseListListInt = p => parseList(p, ~f=parseListInt)

[@progress]
and parseExpression = (~x=4, p: Parser.t) => {
  switch (p.token) {
  | Lparen =>
    Parser.next(p);
    let e1 = parseExpression(p);
    Parser.expect(p, Plus);
    let e2 = parseExpression(p);
    Parser.expect(p, Lparen);
    Expr.Plus(e1, e2);
  | _ => Expr.Int(parseInt(p))
  };
}

[@progress]
and parseListExpression = p => parseList(p, ~f=parseExpression)

[@progress]
and parseListExpression2 = p => parseList(p, ~f=parseExpression(~x=7))

[@progress]
and parseListIntTailRecursive = p => {
  let rec loop = (p: Parser.t, l) =>
    if (p.token == Asterisk) {
      List.rev(l);
    } else {
      loop(p, [parseInt(p), ...l]);
    };
  loop(p, []);
};

[@progress progress]
let rec testLoopAfterProgress = () => {
  progress();
  loopAfterProgress();
}
and loopAfterProgress = () => loopAfterProgress();

module UITermination = {
  type state = int;
  type setState = (~f: state => option(state)) => unit;

  type onClick = unit => unit;
  type dom;

  let nothing: onClick = () => ();

  type div = (~text: string, ~onClick: onClick) => dom;
  let div: div = assert(false);

  let initState = n => n == 0 ? Some(42) : None;
  let increment = n => Some(n + 1);

  let incrementOnClick = (~setState: setState): onClick =>
    () => setState(~f=increment);

  let counter = (state: state, ~setState: setState) => {
    setState(~f=initState);
    div(~text=string_of_int(state), ~onClick=() => setState(~f=increment));
  };

  [@progress initState]
  let rec counterCompiled = (state: state) => {
    switch (initState(state)) {
    | None => ()
    | Some(newState) => ignore(counterCompiled(newState))
    };
    ignore(string_of_int(state));
  }

  and onClick1 = state =>
    switch (increment(state)) {
    | None => ()
    | Some(newState) => counterCompiled(newState)
    };

  let countRenders = (state: state, ~setState: setState) => {
    setState(~f=increment);
    div(
      ~text="I have been rendered " ++ string_of_int(state) ++ " times",
      ~onClick=nothing,
    );
  };

  [@progress initState]
  let rec countRendersCompiled = (state: state) => {
    switch (increment(state)) {
    | None => ()
    | Some(newState) => ignore(countRendersCompiled(newState))
    };
    ignore("I have been rendered " ++ string_of_int(state) ++ " times");
  };
};

module ParserWihtOptionals = {
  let parseListO = (p: Parser.t, ~f) => {
    let rec loop = (p: Parser.t) =>
      if (p.token == Asterisk) {
        [];
      } else {
        switch (f(p)) {
        | None => []
        | Some(item) =>
          let l = loop(p);
          [item, ...l];
        };
      };
    Some(loop(p));
  };

  let parseIntO = (p: Parser.t) => {
    switch (p.token) {
    | Int(n) =>
      Parser.next(p);
      Some(n);
    | _ =>
      Parser.err(p, "integer expected");
      None;
    };
  };

  [@progress (Parser.next, Parser.next)]
  let rec parseListIntO = p => parseListO(p, ~f=parseIntO);
};