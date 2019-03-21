// Generated by BUCKLESCRIPT VERSION 5.0.0, PLEASE EDIT WITH CARE

import * as ImportJsValueGen from "./ImportJsValue.gen";

function round(prim) {
  return ImportJsValueGen.round(prim);
}

function area(prim) {
  return ImportJsValueGen.area(prim);
}

var roundedNumber = ImportJsValueGen.round(1.8);

var areaValue = ImportJsValueGen.area(/* record */[
      /* x */3,
      /* y */undefined
    ]);

function getAbs(x) {
  return x.getAbs();
}

var AbsoluteValue = /* module */[/* getAbs */getAbs];

function useGetProp(x) {
  return x.getProp() + 1 | 0;
}

function useGetAbs(x) {
  return x.getAbs() + 1 | 0;
}

function useColor(prim) {
  return ImportJsValueGen.useColor(prim);
}

function higherOrder(prim) {
  return ImportJsValueGen.higherOrder(prim);
}

var returnedFromHigherOrder = ImportJsValueGen.higherOrder((function (prim, prim$1) {
        return prim + prim$1 | 0;
      }));

function convertVariant(prim) {
  return ImportJsValueGen.convertVariant(prim);
}

export {
  round ,
  area ,
  roundedNumber ,
  areaValue ,
  AbsoluteValue ,
  useGetProp ,
  useGetAbs ,
  useColor ,
  higherOrder ,
  returnedFromHigherOrder ,
  convertVariant ,
  
}
/* roundedNumber Not a pure module */
