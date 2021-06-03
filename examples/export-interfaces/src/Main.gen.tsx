/* TypeScript file generated from Main.res by genType. */
/* eslint-disable import/first */


// @ts-ignore: Implicit any on import
import * as MainBS__Es6Import from './Main.bs';
const MainBS: any = MainBS__Es6Import;

export interface Ibar { readonly baz: number };

export const process: (_1:{ readonly bar: Ibar }) => number = function (Arg1: any) {
  const result = MainBS.process(Arg1.bar);
  return result
};
