// Generated by BUCKLESCRIPT VERSION 5.0.2, PLEASE EDIT WITH CARE

import * as Curry from "bs-platform/lib/es6/curry.js";
import * as React from "react";

function Hooks(Props) {
  var vehicle = Props.vehicle;
  var match = React.useState((function () {
          return 0;
        }));
  var setCount = match[1];
  var count = match[0];
  return React.createElement("div", undefined, React.createElement("p", undefined, "Hooks example " + (vehicle[/* name */0] + (" clicked " + (String(count) + " times")))), React.createElement("button", {
                  onClick: (function (param) {
                      return Curry._1(setCount, (function (param) {
                                    return count + 1 | 0;
                                  }));
                    })
                }, "Click me"));
}

var make = Hooks;

export {
  make ,
  
}
/* react Not a pure module */
