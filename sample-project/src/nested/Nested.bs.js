// Generated by BUCKLESCRIPT VERSION 4.0.3, PLEASE EDIT WITH CARE
'use strict';


function consumeVariant(x) {
  if (typeof x === "number") {
    return 1;
  } else if (x.tag) {
    var n = x[0];
    return (
            n !== undefined ? n : 0
          ) + 3 | 0;
  } else {
    return (x[0] + x[1] | 0) + 2 | 0;
  }
}

exports.consumeVariant = consumeVariant;
/* No side effect */
