import * as React from "react";
import * as ReactDOM from "react-dom";
import App from "./App";
import ComponentAsProp from "./components/ComponentAsProp.gen";
import { InnerComponent } from "./components/ManyComponents.gen";
import * as ImportJsValue from "./ImportJsValue.gen";
import * as Uncurried from "./Uncurried.gen";
import "./index.css";
import * as MyMath from "./MyMath";
import * as Types from "./nested/Types.gen";
import { Universe_Nested2_Nested3_nested3Value } from "./NestedModules.gen";
import ReasonComponent from "./ReasonComponent.gen";
import { minus, t, tToString } from "./ReasonComponent.gen";
import * as Records from "./Records.gen";
import registerServiceWorker from "./registerServiceWorker";
import * as Variants from "./Variants.gen";
import Hooks from "./Hooks.gen";
import {
  printManyPayloads,
  printVariantWithPayload,
  printVariantWithPayloads,
  testManyPayloads,
  testVariantWithPayloads,
  testWithPayload
} from "./VariantsWithPayload.gen";
import * as TestPromise from "./TestPromise.gen";

const minusOne: number = minus({ second: 1 });

const a: t = "A";
const b: t = { tag: "B", value: 3 };

// tslint:disable-next-line:no-console
const consoleLog = console.log;

consoleLog(a, b);

const intList = Types.map(x => x + 1, Types.someIntList);

const businesses = [
  {
    address: "Poison road",
    name: "AcmeLTD",
    owner: { name: "John", age: 12, address: "garage" }
  }
];

const addresses = Records.findAllAddresses(businesses);

consoleLog("index.tsx roundedNumber:", ImportJsValue.roundedNumber);
consoleLog("index.tsx areaValue:", ImportJsValue.areaValue);
consoleLog(
  "index.tsx returnedFromHigherOrder:",
  ImportJsValue.returnedFromHigherOrder
);

consoleLog("index.tsx callback:", Uncurried.callback(() => 3));
consoleLog(
  "index.tsx callback2:",
  Uncurried.callback2({ login: () => "hello" })
);
consoleLog(
  "index.tsx callback2U:",
  Uncurried.callback2U({ loginU: () => "hello" })
);
Uncurried.sumU(3, 4);
Uncurried.sumU2(3)(4);
Uncurried.sumCurried(3, 4);
Uncurried.sumLblCurried("hello", { n: 3, m: 4 });

ReactDOM.render(
  <div>
    <App name={"Hello"} />
    <ReasonComponent
      message={
        "Message from typescript: minus one is " +
        minusOne +
        " and B(3) prints as " +
        tToString(b) +
        " addresses: " +
        addresses
      }
      intList={intList}
      person={{
        name: "Name",
        polymorphicPayload: null,
        surname: "Surname",
        type: ""
      }}
    />
    <InnerComponent />
    <ComponentAsProp
      title={<div>title</div>}
      description={<div>description</div>}
    />
    <Hooks vehicle={{ name: "Car" }} />
  </div>,
  document.getElementById("root") as HTMLElement
);
registerServiceWorker();

const x1 = Records.getPayload(Records.payloadValue).v;
const x2 = Records.getPayloadRecord(Records.payloadValue).v;
const x3 = Records.payloadValue.payload.v;
const x4 = Records.getPayloadRecordPlusOne(Records.payloadValue).v;
consoleLog("x1,x2,x3,x4 are", x1, x2, x3, x4);

consoleLog(
  "Universe_Nested2_Nested3_nested3Value: ",
  Universe_Nested2_Nested3_nested3Value
);

consoleLog("Enums: swap(sunday) =", Variants.swap("sunday"));
consoleLog("Enums: fortytwoOK is", Variants.fortytwoOK);
consoleLog("Enums: fortytwoBAD is", Variants.fortytwoBAD);
consoleLog(
  "Variants: testConvert3to2('module') =",
  Variants.testConvert2to3("module")
);
consoleLog("Variants: testConvert3to2('42') =", Variants.testConvert2to3("42"));

const absoluteValueInstance = new MyMath.AbsoluteValue();
absoluteValueInstance.prop = -3;
consoleLog("absoluteValueInstance", absoluteValueInstance);

const propValue = ImportJsValue.useGetProp(absoluteValueInstance);
const absValue = ImportJsValue.useGetAbs(absoluteValueInstance);
consoleLog("ImportJsValue: getProp() =", propValue);
consoleLog("ImportJsValue: getAbs() =", absValue);

printVariantWithPayload("a");
printVariantWithPayload("bRenamed");
printVariantWithPayload(true);
printVariantWithPayload(20);
printVariantWithPayload(0.5);
printVariantWithPayload(testWithPayload({ x: 15 }));

printManyPayloads({ tag: "oneRenamed", value: 34 });
printManyPayloads({ tag: 2, value: ["hello", "world"] });
printManyPayloads(testManyPayloads({ tag: "three", value: { x: 15 } }));

printVariantWithPayloads(testVariantWithPayloads("ARenamed"));
printVariantWithPayloads(testVariantWithPayloads({ tag: "B", value: 4 }));
printVariantWithPayloads(testVariantWithPayloads({ tag: "C", value: [1, 2] }));
printVariantWithPayloads(testVariantWithPayloads({ tag: "D", value: [1, 2] }));
printVariantWithPayloads(
  testVariantWithPayloads({ tag: "E", value: [1, "hello", 2] })
);

TestPromise.convert(Promise.resolve({ x: 3, s: "hello" })).then(x =>
  consoleLog("TestPromise result:", x.result)
);
