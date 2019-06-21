/* TypeScript file generated by genType. */
/* eslint-disable import/first */


// tslint:disable-next-line:no-var-requires
const Curry = require('bs-platform/lib/es6/curry.js');

// tslint:disable-next-line:no-var-requires
const HooksBS = require('./Hooks.bs');

// tslint:disable-next-line:interface-over-type-literal
export type vehicle = { readonly name: string };

// tslint:disable-next-line:interface-over-type-literal
export type cb = (_1:{ readonly to: vehicle }) => void;

// tslint:disable-next-line:interface-over-type-literal
export type callback<input,output> = (_1:input) => output;

// tslint:disable-next-line:interface-over-type-literal
export type testReactContext = React.Context<number>;

// tslint:disable-next-line:interface-over-type-literal
export type testReactRef = React.Ref<number>;

export const $$default: React.FC<{ readonly vehicle: vehicle }> = function Hooks(Arg1: any) {
  const result = HooksBS.default({vehicle:[Arg1.vehicle.name]});
  return result
};

export default $$default;

export const anotherComponent: React.FC<{ readonly callback: (_1:void) => void; readonly vehicle: vehicle }> = function Hooks_anotherComponent(Arg1: any) {
  const result = HooksBS.anotherComponent({callback:Arg1.callback, vehicle:[Arg1.vehicle.name]});
  return result
};

export const Inner_make: React.FC<{ readonly vehicle: vehicle }> = function Hooks_Inner(Arg1: any) {
  const result = HooksBS.Inner[0]({vehicle:[Arg1.vehicle.name]});
  return result
};

export const Inner_anotherComponent: React.FC<{ readonly vehicle: vehicle }> = function Hooks_Inner_anotherComponent(Arg1: any) {
  const result = HooksBS.Inner[1]({vehicle:[Arg1.vehicle.name]});
  return result
};

export const Inner_Inner2_make: React.FC<{ readonly vehicle: vehicle }> = function Hooks_Inner_Inner2(Arg1: any) {
  const result = HooksBS.Inner[2][0]({vehicle:[Arg1.vehicle.name]});
  return result
};

export const Inner_Inner2_anotherComponent: React.FC<{ readonly vehicle: vehicle }> = function Hooks_Inner_Inner2_anotherComponent(Arg1: any) {
  const result = HooksBS.Inner[2][1]({vehicle:[Arg1.vehicle.name]});
  return result
};

export const NoProps_make: React.FC<{}> = HooksBS.NoProps[0];

export const functionWithRenamedArgs: (_1:{ readonly to: vehicle; readonly Type: vehicle }, _2:cb) => string = function (Arg1: any, Arg2: any) {
  const result = Curry._3(HooksBS.functionWithRenamedArgs, [Arg1.to.name], [Arg1.Type.name], function (Argto: any) {
      const result1 = Arg2({to:{name:Argto[0]}});
      return result1
    });
  return result
};

export const componentWithRenamedArgs: (_1:{ readonly Type: vehicle; readonly to: vehicle }, _2:cb) => JSX.Element = function (Arg1: any, Arg2: any) {
  const result = Curry._2(HooksBS.componentWithRenamedArgs, {Type:[Arg1.Type.name], to:[Arg1.to.name]}, function (Argto: any) {
      const result1 = Arg2({to:{name:Argto[0]}});
      return result1
    });
  return result
};

export const makeWithRef: (_1:{ readonly vehicle: vehicle }, _2:(null | undefined | any)) => JSX.Element = function (Arg1: any, Arg2: any) {
  const result = Curry._2(HooksBS.makeWithRef, {vehicle:[Arg1.vehicle.name]}, Arg2);
  return result
};

export const testForwardRef: React.FC<{ readonly vehicle: vehicle }> = function Hooks_testForwardRef(Arg1: any) {
  const result = HooksBS.testForwardRef({vehicle:[Arg1.vehicle.name]});
  return result
};

export const polymorphicComponent: React.FC<{ readonly p: [vehicle, any] }> = function Hooks_polymorphicComponent<T1>(Arg1: any) {
  const result = HooksBS.polymorphicComponent({p:[[Arg1.p[0].name], Arg1.p[1]]});
  return result
};

export const functionReturningReactElement: React.FC<{ readonly name: string }> = HooksBS.functionReturningReactElement;
