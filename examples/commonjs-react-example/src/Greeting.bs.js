// Generated by BUCKLESCRIPT VERSION 5.0.0-dev.5, PLEASE EDIT WITH CARE
'use strict';

var React = require("react");
var ReasonReact = require("reason-react/src/ReasonReact.js");
var ImportMyBanner$CommonjsReactExample = require("./ImportMyBanner.bs.js");

var component = ReasonReact.statelessComponent("PageReason");

function onClick(param) {
  console.log("click");
  return /* () */0;
}

function make(message, someNumber, extraGreeting, _children) {
  return /* record */[
          /* debugName */component[/* debugName */0],
          /* reactClassInternal */component[/* reactClassInternal */1],
          /* handedOffState */component[/* handedOffState */2],
          /* willReceiveProps */component[/* willReceiveProps */3],
          /* didMount */component[/* didMount */4],
          /* didUpdate */component[/* didUpdate */5],
          /* willUnmount */component[/* willUnmount */6],
          /* willUpdate */component[/* willUpdate */7],
          /* shouldUpdate */component[/* shouldUpdate */8],
          /* render */(function (_self) {
              var greeting = extraGreeting !== undefined ? extraGreeting : "How are you?";
              return React.createElement("div", {
                          onClick: onClick
                        }, ReasonReact.element(undefined, undefined, ImportMyBanner$CommonjsReactExample.make(true, message + (" " + greeting), /* array */[])), "someNumber:" + String(someNumber));
            }),
          /* initialState */component[/* initialState */10],
          /* retainedProps */component[/* retainedProps */11],
          /* reducer */component[/* reducer */12],
          /* jsElementWrapped */component[/* jsElementWrapped */13]
        ];
}

function testBike(x) {
  return x;
}

exports.component = component;
exports.onClick = onClick;
exports.make = make;
exports.testBike = testBike;
/* component Not a pure module */
