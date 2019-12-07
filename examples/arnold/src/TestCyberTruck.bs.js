// Generated by BUCKLESCRIPT, PLEASE EDIT WITH CARE

import * as Block from "bs-platform/lib/es6/block.js";
import * as Curry from "bs-platform/lib/es6/curry.js";
import * as Random from "bs-platform/lib/es6/random.js";
import * as Caml_obj from "bs-platform/lib/es6/caml_obj.js";
import * as Caml_builtin_exceptions from "bs-platform/lib/es6/caml_builtin_exceptions.js";

var counter = {
  contents: Random.$$int(100)
};

function progress(param) {
  if (counter.contents < 0) {
    throw [
          Caml_builtin_exceptions.assert_failure,
          /* tuple */[
            "TestCyberTruck.re",
            5,
            22
          ]
        ];
  }
  counter.contents = counter.contents - 1 | 0;
  return /* () */0;
}

var Nested = {
  f: progress
};

var Progress = {
  Nested: Nested
};

function justReturn(param) {
  return /* () */0;
}

function alwaysLoop(_param) {
  while(true) {
    _param = /* () */0;
    continue ;
  };
}

function alwaysProgress(_param) {
  while(true) {
    progress(/* () */0);
    _param = /* () */0;
    continue ;
  };
}

function alwaysProgressWrongOrder(param) {
  alwaysProgressWrongOrder(/* () */0);
  return progress(/* () */0);
}

function doNotAlias(_param) {
  while(true) {
    _param = /* () */0;
    continue ;
  };
}

function progressOnBothBranches(x) {
  while(true) {
    progress(/* () */0);
    continue ;
  };
}

function progressOnOneBranch(x) {
  while(true) {
    if (x > 3) {
      progress(/* () */0);
    }
    continue ;
  };
}

function testParametricFunction(x) {
  while(true) {
    if (x > 3) {
      progress(/* () */0);
    }
    continue ;
  };
}

var testParametricFunction2 = testParametricFunction;

function callParseFunction(x, parseFunction) {
  return Curry._1(parseFunction, x);
}

function testCacheHit(x) {
  while(true) {
    if (x > 0) {
      progress(/* () */0);
    }
    continue ;
  };
}

function doNothing(param) {
  return /* () */0;
}

function evalOrderIsNotLeftToRight(x) {
  evalOrderIsNotLeftToRight(x);
  progress(/* () */0);
  return /* () */0;
}

function evalOrderIsNotRightToLeft(x) {
  progress(/* () */0);
  evalOrderIsNotRightToLeft(x);
  return /* () */0;
}

function butFirstArgumentIsAlwaysEvaluated(x) {
  while(true) {
    progress(/* () */0);
    continue ;
  };
}

function butSecondArgumentIsAlwaysEvaluated(x) {
  while(true) {
    progress(/* () */0);
    continue ;
  };
}

function tokenToString(token) {
  if (typeof token === "number") {
    switch (token) {
      case /* Asterisk */0 :
          return "*";
      case /* Eof */1 :
          return "Eof";
      case /* Lparen */2 :
          return "(";
      case /* Plus */3 :
          return "+";
      case /* Rparen */4 :
          return ")";
      
    }
  } else {
    return String(token[0]);
  }
}

function next(p) {
  var match = Random.bool(/* () */0);
  p.token = match ? /* Eof */1 : /* Int */[Random.$$int(1000)];
  p.position = {
    lnum: Random.$$int(1000),
    cnum: Random.$$int(80)
  };
  return /* () */0;
}

function err(p, s) {
  p.errors = /* :: */[
    s,
    p.errors
  ];
  return /* () */0;
}

function expect(p, token) {
  if (Caml_obj.caml_equal(p.token, token)) {
    return next(p);
  } else {
    return err(p, "expected token " + tokenToString(p.token));
  }
}

var Parser = {
  tokenToString: tokenToString,
  next: next,
  err: err,
  expect: expect
};

var Expr = { };

function parseList(p, f) {
  var loop = function (p) {
    if (p.token === /* Asterisk */0) {
      return /* [] */0;
    } else {
      var item = Curry._1(f, p);
      var l = loop(p);
      return /* :: */[
              item,
              l
            ];
    }
  };
  return loop(p);
}

function $$parseInt(p) {
  var match = p.token;
  var res = typeof match === "number" ? (err(p, "integer expected"), -1) : match[0];
  next(p);
  return res;
}

function parseExpression($staropt$star, p) {
  var match = p.token;
  if (typeof match === "number" && match === 2) {
    next(p);
    var e1 = parseExpression(undefined, p);
    expect(p, /* Plus */3);
    var e2 = parseExpression(undefined, p);
    expect(p, /* Lparen */2);
    return /* Plus */Block.__(1, [
              e1,
              e2
            ]);
  } else {
    return /* Int */Block.__(0, [$$parseInt(p)]);
  }
}

function parseListInt(p) {
  return parseList(p, $$parseInt);
}

function parseListListInt(p) {
  return parseList(p, parseListInt);
}

function parseListExpression(p) {
  return parseList(p, (function (eta) {
                return parseExpression(undefined, eta);
              }));
}

function parseListExpression2(p) {
  var partial_arg = 7;
  return parseList(p, (function (param) {
                return parseExpression(partial_arg, param);
              }));
}

var progress2 = progress;

export {
  progress ,
  progress2 ,
  Progress ,
  justReturn ,
  alwaysLoop ,
  alwaysProgress ,
  alwaysProgressWrongOrder ,
  doNotAlias ,
  progressOnBothBranches ,
  progressOnOneBranch ,
  testParametricFunction ,
  testParametricFunction2 ,
  callParseFunction ,
  testCacheHit ,
  doNothing ,
  evalOrderIsNotLeftToRight ,
  evalOrderIsNotRightToLeft ,
  butFirstArgumentIsAlwaysEvaluated ,
  butSecondArgumentIsAlwaysEvaluated ,
  Parser ,
  Expr ,
  parseList ,
  parseListInt ,
  parseListListInt ,
  $$parseInt ,
  parseExpression ,
  parseListExpression ,
  parseListExpression2 ,
  
}
/* counter Not a pure module */
