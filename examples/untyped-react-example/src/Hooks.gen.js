/*
 * this is a custom header you can add to every file!
 */
import * as PropTypes from 'prop-types';

import * as HooksBS from './Hooks.bs';

export const $$default = function Hooks(Arg1) {
  const result = HooksBS.default({vehicle:[Arg1.vehicle.name]});
  return result
};

$$default.propTypes = {
  vehicle : PropTypes.shape({
    name : PropTypes.string.isRequired
  }).isRequired
};

export default $$default;
