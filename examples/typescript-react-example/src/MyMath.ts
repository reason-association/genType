/* @flow strict */

export const round: ((_: number) => number) = Math.round;

// tslint:disable-next-line:only-arrow-functions
export const area = function(point: { x: number; y?: number }): number {
  return point.x * (point.y === undefined ? 1 : point.y);
};

export class AbsoluteValue {
  public prop!: number;
  public getProp(): number {
    return this.prop;
  }
  public getAbs(): number {
    return this.prop < 0 ? -this.prop : this.prop;
  }
}

export type stringFunction = (_: string) => string;

// tslint:disable-next-line:only-arrow-functions
export const useColor = function(x: "tomato" | "gray"): number {
  return 0;
};


export const higherOrder = (foo: (_1:number, _2:number) => number) => foo(3,4);

export const convertVariant = (x:any) => x;

export const polymorphic = <T>(x:T) : T => x;

export default 34;