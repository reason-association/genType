/** 
 * @flow strict
 * @generated 
 * @nolint
 */

// $FlowExpectedError: Reason checked type sufficiently
const Component3BS = require('./Component3.bs');

// $FlowExpectedError: Reason checked type sufficiently
const ReasonReact = require('reason-react/src/ReasonReact.js');

export type Props = {|+children?: mixed|};

export const component: React$ComponentType<Props> = ReasonReact.wrapReasonForJs(
  Component3BS.component,
  (function _(jsProps: Props) {
     return Component3BS.make(jsProps.children);
  }));

export default component;
