/* @flow strict */

const ReactDOM = require("react-dom");
const React = require("react");

const GreetingRe = require("./Greeting.re");

// Import a ReasonReact component!
const PageReason = require("./Greeting.re").default;

const helloWorldList = GreetingRe.cons({
  x: "Hello",
  l: GreetingRe.cons2({ x: "World", l: GreetingRe.empty })
});

const helloWorld = GreetingRe.concat("++", helloWorldList);

const someNumber: number = GreetingRe.testDefaultArgs({ y: 10 });

const TestRE = require("./Test.re");
console.log("interopRoot.js roundedNumber:", TestRE.roundedNumber);

const App = () => (
  <div>
    <PageReason
      message={helloWorld}
      someNumber={someNumber}
      polymorphicProp={[1, 2, 3]}
    />
  </div>
);
App.displayName = "ExampleInteropRoot";

// $FlowExpectedError: Reason checked type sufficiently
ReactDOM.render(React.createElement(App), document.getElementById("index"));
