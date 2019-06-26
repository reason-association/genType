// Generated by BUCKLESCRIPT VERSION 5.0.5, PLEASE EDIT WITH CARE

import * as Curry from "bs-platform/lib/es6/curry.js";

function uncurried0() {
  return "";
}

function uncurried1(x) {
  return String(x);
}

function uncurried2(x, y) {
  return String(x) + y;
}

function uncurried3(x, y, z) {
  return String(x) + (y + String(z));
}

function curried3(x, y, z) {
  return String(x) + (y + String(z));
}

function callback(cb) {
  return String(Curry._1(cb, /* () */0));
}

function callback2(auth) {
  return Curry._1(auth[/* login */0], /* () */0);
}

function callback2U(auth) {
  return auth[/* loginU */0]();
}

function sumU(n, m) {
  console.log("sumU 2nd arg", m, "result", n + m | 0);
  return /* () */0;
}

function sumU2(n) {
  return (function (m) {
      console.log("sumU2 2nd arg", m, "result", n + m | 0);
      return /* () */0;
    });
}

function sumCurried(n) {
  console.log("sumCurried 1st arg", n);
  return (function (m) {
      console.log("sumCurried 2nd arg", m, "result", n + m | 0);
      return /* () */0;
    });
}

function sumLblCurried(s, n) {
  console.log(s, "sumLblCurried 1st arg", n);
  return (function (m) {
      console.log("sumLblCurried 2nd arg", m, "result", n + m | 0);
      return /* () */0;
    });
}

export {
  uncurried0 ,
  uncurried1 ,
  uncurried2 ,
  uncurried3 ,
  curried3 ,
  callback ,
  callback2 ,
  callback2U ,
  sumU ,
  sumU2 ,
  sumCurried ,
  sumLblCurried ,
  
}
/* No side effect */
