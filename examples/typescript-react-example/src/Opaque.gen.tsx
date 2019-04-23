/* TypeScript file generated by genType. */

// tslint:disable-next-line:no-var-requires
const OpaqueBS = require('./Opaque.bs');

import {business as Records_business} from './Records.gen';

// tslint:disable-next-line:max-classes-per-file 
// tslint:disable-next-line:class-name
export abstract class opaqueFromRecords { protected opaque!: any }; /* simulate opaque types */

// tslint:disable-next-line:interface-over-type-literal
export type pair = [opaqueFromRecords, opaqueFromRecords];

export const noConversion: (_1:opaqueFromRecords) => opaqueFromRecords = OpaqueBS.noConversion;

export const testConvertNestedRecordFromOtherFile: (_1:Records_business) => Records_business = function (Arg1: any) {
  const result = OpaqueBS.testConvertNestedRecordFromOtherFile([Arg1.name, (Arg1.owner == null ? undefined : [Arg1.owner.name, Arg1.owner.age, Arg1.owner.address]), Arg1.address]);
  return {name:result[0], owner:(result[1] == null ? result[1] : {name:result[1][0], age:result[1][1], address:result[1][2]}), address:result[2]}
};
