[@genType]
type weekday = [
  | `monday
  | `tuesday
  | `wednesday
  | `thursday
  | `friday
  | `saturday
  | `sunday
];

[@genType]
let isWeekend = (x: weekday) =>
  switch (x) {
  | `saturday
  | `sunday => true
  | _ => false
  };

[@genType]
let monday = `monday;
[@genType]
let saturday = `saturday;
[@genType]
let sunday = `sunday;

[@genType]
let onlySunday = (_: [ | `sunday]) => ();

[@genType]
let swap = x =>
  switch (x) {
  | `sunday => `saturday
  | `saturday => `sunday
  };

[@genType]
type testGenTypeAs = [
  | [@genType.as "type"] `type_
  | [@genType.as "module"] `module_
  | [@genType.as "42"] `fortytwo
];

[@genType]
let testConvert = (x: testGenTypeAs) => x;

[@genType]
let fortytwoOK: testGenTypeAs = `fortytwo;

/* Exporting this is BAD: type inference means it's not mapped to "42" */
[@genType]
let fortytwoBAD = `fortytwo;

[@genType]
type testGenTypeAs2 = [
  | [@genType.as "type"] `type_
  | [@genType.as "module"] `module_
  | [@genType.as "42"] `fortytwo
];

/* Since testGenTypeAs2 is the same type as testGenTypeAs1,
   share the conversion map. */
[@genType]
let testConvert2 = (x: testGenTypeAs2) => x;

[@genType]
type testGenTypeAs3 = [
  | [@genType.as "type"] `type_
  | [@genType.as "module"] `module_
  | [@genType.as "XXX THIS IS DIFFERENT"] `fortytwo
];

/* Since testGenTypeAs3 has a different representation:
   use a new conversion map. */
[@genType]
let testConvert3 = (x: testGenTypeAs3) => x;

/* This converts between testGenTypeAs2 and testGenTypeAs3 */
[@genType]
let testConvert2to3 = (x: testGenTypeAs2): testGenTypeAs3 => x;

[@genType]
type x1 = [ | `x | [@genType.as "same"] `x1];

[@genType]
type x2 = [ | `x | [@genType.as "same"] `x2];

[@genType]
let id1 = (x: x1) => x;

[@genType]
let id2 = (x: x2) => x;

[@genType]
[@genType.as "type"]
type type_ =
  | [@dead "type_.Type"] [@genType.as "type"] Type;