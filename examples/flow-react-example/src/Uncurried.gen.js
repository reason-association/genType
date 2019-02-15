/** 
 * @flow strict
 * @generated
 * @nolint
 */
/* eslint-disable */

// $FlowExpectedError: Reason checked type sufficiently
import * as Curry from 'bs-platform/lib/es6/curry.js';

// $FlowExpectedError: Reason checked type sufficiently
import * as UncurriedBS from './Uncurried.bs';

export const uncurried0: () => string = UncurriedBS.uncurried0;

export const uncurried1: (number) => string = UncurriedBS.uncurried1;

export const uncurried2: (number, string) => string = UncurriedBS.uncurried2;

export const uncurried3: (number, string, number) => string = UncurriedBS.uncurried3;

export const curried3: (number, string, number) => string = function _(Arg1, Arg2, Arg3) {
  const result = Curry._3(UncurriedBS.curried3, Arg1, Arg2, Arg3);
  return result
};
