// Generated by BUCKLESCRIPT VERSION 5.0.2, PLEASE EDIT WITH CARE

import * as Curry from "bs-platform/lib/es6/curry.js";
import * as React from "react";
import * as Caml_array from "bs-platform/lib/es6/caml_array.js";
import * as ReasonReact from "reason-react/src/ReasonReact.js";

var component = ReasonReact.statelessComponent("App");

function make(array, $staropt$star, person, title, _children) {
  var callback = $staropt$star !== undefined ? $staropt$star : (function (param) {
        return /* () */0;
      });
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
              Curry._1(callback, /* () */0);
              return React.createElement("div", undefined, "Test Component Title:" + (title + (" Name:" + (person[/* name */0] + (" array[0]:" + Caml_array.caml_array_get(array, 0))))));
            }),
          /* initialState */component[/* initialState */10],
          /* retainedProps */component[/* retainedProps */11],
          /* reducer */component[/* reducer */12],
          /* jsElementWrapped */component[/* jsElementWrapped */13]
        ];
}

export {
  component ,
  make ,
  
}
/* component Not a pure module */
