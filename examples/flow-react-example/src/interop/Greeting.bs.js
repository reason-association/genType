// Generated by BUCKLESCRIPT, PLEASE EDIT WITH CARE

import * as Curry from "bs-platform/lib/es6/curry.js";
import * as React from "react";
import * as $$String from "bs-platform/lib/es6/string.js";
import * as ReasonReact from "reason-react/src/ReasonReact.js";
import * as ImportMyBanner from "./ImportMyBanner.bs.js";

var component = ReasonReact.statelessComponent("PageReason");

function onClick(param) {
  console.log("click");
  return /* () */0;
}

function make(message, someNumber, extraGreeting, polymorphicProp, _children) {
  return {
          debugName: component.debugName,
          reactClassInternal: component.reactClassInternal,
          handedOffState: component.handedOffState,
          willReceiveProps: component.willReceiveProps,
          didMount: component.didMount,
          didUpdate: component.didUpdate,
          willUnmount: component.willUnmount,
          willUpdate: component.willUpdate,
          shouldUpdate: component.shouldUpdate,
          render: (function (_self) {
              console.log("polymorphicProp:", polymorphicProp);
              var greeting = extraGreeting !== undefined ? extraGreeting : "How are you?";
              return React.createElement("div", {
                          onClick: onClick
                        }, ReasonReact.element(undefined, undefined, ImportMyBanner.make(true, message + (" " + greeting), [])), "someNumber:" + String(someNumber));
            }),
          initialState: component.initialState,
          retainedProps: component.retainedProps,
          reducer: component.reducer,
          jsElementWrapped: component.jsElementWrapped
        };
}

function cons(x, l) {
  return /* :: */[
          x,
          l
        ];
}

function cons2(l, x) {
  return /* :: */[
          x,
          l
        ];
}

function testNamedArgs(a, b, x, c, d, y, e) {
  return (((((a + b | 0) + x | 0) + c | 0) + d | 0) + y | 0) + e | 0;
}

function testCallNamedArgs(foo, a, b) {
  return Curry._2(foo, a, b);
}

function testDefaultArgs($staropt$star, y) {
  var x = $staropt$star !== undefined ? $staropt$star : 3;
  return x + y | 0;
}

function testDefaultArgsWithRecordConversion(param, param$1) {
  return 34;
}

function testDefaultArgsWithVariantConversion(param, param$1) {
  return 34;
}

function testDefaultArgsWithVariantConversionContravariant(param, param$1) {
  return 34;
}

var empty = /* [] */0;

var concat = $$String.concat;

export {
  component ,
  onClick ,
  make ,
  empty ,
  cons ,
  cons2 ,
  concat ,
  testNamedArgs ,
  testCallNamedArgs ,
  testDefaultArgs ,
  testDefaultArgsWithRecordConversion ,
  testDefaultArgsWithVariantConversion ,
  testDefaultArgsWithVariantConversionContravariant ,
  
}
/* component Not a pure module */
