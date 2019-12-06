/* TypeScript file generated from Types.re by genType. */
/* eslint-disable import/first */


// tslint:disable-next-line:no-var-requires
const Curry = require('bs-platform/lib/es6/curry.js');

// tslint:disable-next-line:no-var-requires
const TypesBS = require('./Types.bs');

import {Dict_t as Js_Dict_t} from '../../src/shims/Js.shim';

import {Json_t as Js_Json_t} from '../../src/shims/Js.shim';

import {list} from '../../src/shims/ReasonPervasives.shim';

import {t as Location_t} from '../../src/location/location.gen';

// tslint:disable-next-line:interface-over-type-literal
export type t = number;

// tslint:disable-next-line:interface-over-type-literal
export type typeWithVars<x,y,z> = 
    { tag: "A"; value: [x, y] }
  | { tag: "B"; value: z };

// tslint:disable-next-line:interface-over-type-literal
export type tree = {
  readonly label: string; 
  readonly left?: tree; 
  readonly right?: tree
};

// tslint:disable-next-line:interface-over-type-literal
export type selfRecursive = { readonly self: selfRecursive };

// tslint:disable-next-line:interface-over-type-literal
export type mutuallyRecursiveA = { readonly b: mutuallyRecursiveB };

// tslint:disable-next-line:interface-over-type-literal
export type mutuallyRecursiveB = { readonly a: mutuallyRecursiveA };

// tslint:disable-next-line:max-classes-per-file 
// tslint:disable-next-line:class-name
export abstract class opaqueVariant { protected opaque!: any }; /* simulate opaque types */

// tslint:disable-next-line:interface-over-type-literal
export type twice<a> = [a, a];

// tslint:disable-next-line:interface-over-type-literal
export type genTypeMispelled = number;

// tslint:disable-next-line:interface-over-type-literal
export type dictString = Js_Dict_t<string>;

// tslint:disable-next-line:interface-over-type-literal
export type nullOrString = (null | string);

// tslint:disable-next-line:interface-over-type-literal
export type nullOrString2 = (null | string);

// tslint:disable-next-line:interface-over-type-literal
export type record = { readonly i: number; readonly s: string };

// tslint:disable-next-line:interface-over-type-literal
export type decorator<a,b> = (_1:a) => b;

// tslint:disable-next-line:interface-over-type-literal
export type marshalFields = {
  readonly rec: string; 
  readonly _switch: string; 
  readonly switch: string; 
  readonly __: string; 
  readonly _: string; 
  readonly foo: string; 
  readonly _foo: string; 
  readonly Uppercase: string; 
  readonly _Uppercase: string
};

// tslint:disable-next-line:interface-over-type-literal
export type marshalMutableField = { match: number };

// tslint:disable-next-line:interface-over-type-literal
export type ocaml_array<a> = a[];

// tslint:disable-next-line:interface-over-type-literal
export type someRecord = { readonly id: number };

// tslint:disable-next-line:interface-over-type-literal
export type instantiateTypeParameter = ocaml_array<someRecord>;

export const someIntList: list<number> = TypesBS.someIntList;

export const map: <T1,T2>(_1:((_1:T1) => T2), _2:list<T1>) => list<T2> = function <T1,T2>(Arg1: any, Arg2: any) {
  const result = Curry._2(TypesBS.map, Arg1, Arg2);
  return result
};

export const swap: (_1:tree) => tree = TypesBS.swap;

export const selfRecursiveConverter: (_1:selfRecursive) => selfRecursive = TypesBS.selfRecursiveConverter;

export const mutuallyRecursiveConverter: (_1:mutuallyRecursiveA) => mutuallyRecursiveB = TypesBS.mutuallyRecursiveConverter;

export const testFunctionOnOptionsAsArgument: <T1,a>(_1:(null | undefined | a), _2:((_1:(null | undefined | a)) => T1)) => T1 = function <T1,a>(Arg1: any, Arg2: any) {
  const result = Curry._2(TypesBS.testFunctionOnOptionsAsArgument, (Arg1 == null ? undefined : Arg1), Arg2);
  return result
};

export const stringT: string = TypesBS.stringT;

export const jsStringT: string = TypesBS.jsStringT;

export const jsString2T: string = TypesBS.jsString2T;

export const jsonStringify: (_1:Js_Json_t) => string = TypesBS.jsonStringify;

export const testConvertNull: (_1:(null | record)) => (null | record) = TypesBS.testConvertNull;

export const testConvertLocation: (_1:Location_t) => Location_t = function (Arg1: any) {
  const result = TypesBS.testConvertLocation([Arg1.id, Arg1.name]);
  return {id:result[0], name:result[1]}
};

export const setMatch: (_1:marshalMutableField) => void = TypesBS.setMatch;

export const testInstantiateTypeParameter: (_1:instantiateTypeParameter) => instantiateTypeParameter = TypesBS.testInstantiateTypeParameter;
