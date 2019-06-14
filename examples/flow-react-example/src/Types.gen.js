/** 
 * @flow strict
 * @generated
 * @nolint
 */
/* eslint-disable */
// $FlowExpectedError: Reason checked type sufficiently
type $any = any;

// flowlint-next-line nonstrict-import:off
import {foo as fooNotChecked} from './name-with-dashes';

// $FlowExpectedError: Reason checked type sufficiently
import * as CreateBucklescriptBlock from 'bs-platform/lib/es6/block.js';

// $FlowExpectedError: Reason checked type sufficiently
import * as Curry from 'bs-platform/lib/es6/curry.js';

// In case of type error, check the type of 'foo' in 'Types.re' and './name-with-dashes'.
export const fooTypeChecked: (number) => number = fooNotChecked;

// Export 'foo' early to allow circular import from the '.bs.js' file.
export const foo: mixed = fooTypeChecked;

// $FlowExpectedError: Reason checked type sufficiently
const TypesBS = require('./Types.bs');

// flowlint-next-line nonstrict-import:off
import type {anInterestingFlowType} from './SomeFlowTypes';

// flowlint-next-line nonstrict-import:off
import type {annotatedVariant as AutoAnnotate_annotatedVariant} from './AutoAnnotate.gen';

// flowlint-next-line nonstrict-import:off
import type {t as Obj_t} from '../src/shims/Obj.shim';

// flowlint-next-line nonstrict-import:off
import type {variant as AutoAnnotate_variant} from './AutoAnnotate.gen';

// flowlint-next-line nonstrict-import:off
import type {weekday} from './SomeFlowTypes';

export type typeWithVars<x,y,z> = 
  | {| tag: "A", value: [x, y] |}
  | {| tag: "B", value: z |};

export type optionInt = ?number;

export type funType = (number) => number;

export type myFloat = number;

export type arrayOfStrings1 = Array<string>;

export type arrayOfStrings2 = Array<string>;

export type maybeString = ?string;

export type maybeString2 = ?string;

export type peopleArray = Array<{| +name: string, +nickname: ?string |}>;

export type myObj = Obj_t;

export type { anInterestingFlowType };

export type { weekday };

export type someMutableFields = {|
  mutable0: string, 
  +immutable: number, 
  mutable1: string, 
  mutable2: string
|};

export opaque type exportOpaqueFromVariants = mixed;

export type dateKey = string;
export type DateKey = dateKey;

export opaque type dateKeyOpaque = mixed;
export type DateKeyOpaque = dateKeyOpaque;

export opaque type opaqueVariant = mixed;

export type filter = {| +name: string, +values: Array<string> |};
export type Filter = filter;

export type twice<a> = [a, a];

export type gadt = "F";

export type objectWithCallback = {| +y?: {| +z?: (void) => number |}, +x?: (void) => number |};

export type ocaml_array<a> = Array<a>;

export type someRecord = {| +id: number |};

export type instantiateTypeParameter = ocaml_array<someRecord>;

export const consumeOption: (?number) => number = function (Arg1: $any) {
  const result = TypesBS.consumeOption((Arg1 == null ? undefined : Arg1));
  return result
};

export const consumeOption2: (optionInt) => number = function (Arg1: $any) {
  const result = TypesBS.consumeOption2((Arg1 == null ? undefined : Arg1));
  return result
};

export const testArray: (Array<?number>) => Array<?number> = function (Arg1: $any) {
  const result = TypesBS.testArray(Arg1.map(function _element(ArrayItem: $any) { return (ArrayItem == null ? undefined : ArrayItem)}));
  return result
};

export const identity: (anInterestingFlowType) => anInterestingFlowType = TypesBS.identity;

export const isWeekend: (weekday) => boolean = TypesBS.isWeekend;

export const testFunctionOnOptionsAsArgument: <T1,a>(?a, ((?a) => T1)) => T1 = function <T1,a>(Arg1: $any, Arg2: $any) {
  const result = Curry._2(TypesBS.testFunctionOnOptionsAsArgument, (Arg1 == null ? undefined : Arg1), Arg2);
  return result
};

export const testDateKey: (dateKey) => dateKey = TypesBS.testDateKey;

export const testAutoAnnotateVariants: (AutoAnnotate_variant) => AutoAnnotate_variant = TypesBS.testAutoAnnotateVariants;

export const testAutoAnnotateVariants2: (AutoAnnotate_annotatedVariant) => AutoAnnotate_annotatedVariant = function (Arg1: $any) {
  const result = TypesBS.testAutoAnnotateVariants2(Arg1.tag==="R2"
    ? CreateBucklescriptBlock.__(0, Arg1.value)
    : CreateBucklescriptBlock.__(1, [Arg1.value]));
  return result.tag===0
    ? {tag:"R2", value:result.slice()}
    : {tag:"R4", value:result[0]}
};

export const convertObjectWithCallback: (objectWithCallback) => objectWithCallback = TypesBS.convertObjectWithCallback;

export const testInstantiateTypeParameter: (instantiateTypeParameter) => instantiateTypeParameter = function (Arg1: $any) {
  const result = TypesBS.testInstantiateTypeParameter(Arg1.map(function _element(ArrayItem: $any) { return [ArrayItem.id]}));
  return result.map(function _element(ArrayItem1: $any) { return {id:ArrayItem1[0]}})
};
