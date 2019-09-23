// Generated by BUCKLESCRIPT, PLEASE EDIT WITH CARE

import * as List from "bs-platform/lib/es6/list.js";
import * as Curry from "bs-platform/lib/es6/curry.js";
import * as Belt_Option from "bs-platform/lib/es6/belt_Option.js";

function swap(tree) {
  return {
          label: tree.label,
          left: Belt_Option.map(tree.right, swap),
          right: Belt_Option.map(tree.left, swap)
        };
}

function selfRecursiveConverter(param) {
  return param[/* self */0];
}

function mutuallyRecursiveConverter(param) {
  return param[/* b */0];
}

function testFunctionOnOptionsAsArgument(a, foo) {
  return Curry._1(foo, a);
}

function jsonStringify(prim) {
  return JSON.stringify(prim);
}

function testConvertNull(x) {
  return x;
}

function testConvertLocation(x) {
  return x;
}

var testMarshalFields = {
  rec: "rec",
  _switch: "_switch",
  switch: "switch",
  __: "__",
  _: "_",
  foo: "foo",
  _foo: "_foo",
  Uppercase: "Uppercase",
  _Uppercase: "_Uppercase"
};

function setMatch(x) {
  x.match = 34;
  return /* () */0;
}

function testInstantiateTypeParameter(x) {
  return x;
}

var someIntList = /* :: */[
  1,
  /* :: */[
    2,
    /* :: */[
      3,
      /* [] */0
    ]
  ]
];

var map = List.map;

var stringT = "a";

var jsStringT = "a";

export {
  someIntList ,
  map ,
  swap ,
  selfRecursiveConverter ,
  mutuallyRecursiveConverter ,
  testFunctionOnOptionsAsArgument ,
  stringT ,
  jsStringT ,
  jsonStringify ,
  testConvertNull ,
  testConvertLocation ,
  testMarshalFields ,
  setMatch ,
  testInstantiateTypeParameter ,
  
}
/* No side effect */
