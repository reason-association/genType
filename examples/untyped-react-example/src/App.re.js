/* Untyped file generated by genFlow. */

const AppBS = require("./App.bs");
const ReasonReact = require("reason-react/src/ReasonReact.js");

export const App = ReasonReact.wrapReasonForJs(
  AppBS.component,
  (function _(jsProps) {
     return AppBS.make(jsProps.children);
  }));
export default App;