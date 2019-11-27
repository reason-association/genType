/** 
 * @flow strict
 * @generated from References.re
 * @nolint
 */
/* eslint-disable */
// $FlowExpectedError: Reason checked type sufficiently
type $any = any;

// $FlowExpectedError: Reason checked type sufficiently
import * as Curry from 'bs-platform/lib/es6/curry.js';

// $FlowExpectedError: Reason checked type sufficiently
import * as ReferencesBS from './References.bs';

export opaque type R_t<a> = mixed;

export type t<a> = R_t<a>;

export type requiresConversion = {| +x: number |};

export const create: (number) => [number] = ReferencesBS.create;

export const access: ([number]) => number = ReferencesBS.access;

export const update: ([number]) => void = ReferencesBS.update;

export const get: <T1>(R_t<T1>) => T1 = ReferencesBS.get;

export const make: <T1>(T1) => R_t<T1> = ReferencesBS.make;

export const set: <T1>(R_t<T1>, T1) => void = function <T1>(Arg1: $any, Arg2: $any) {
  const result = Curry._2(ReferencesBS.set, Arg1, Arg2);
  return result
};

export const destroysRefIdentity: ([requiresConversion]) => [requiresConversion] = function (Arg1: $any) {
  const result = ReferencesBS.destroysRefIdentity([[Arg1[0].x]]);
  return [{x:result[0][0]}]
};

export const preserveRefIdentity: (R_t<requiresConversion>) => R_t<requiresConversion> = ReferencesBS.preserveRefIdentity;
