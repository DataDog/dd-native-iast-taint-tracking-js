const path = require('path');
const assert = require('assert');
const iastnativemethods = require(path.join(__dirname, '..', '..', 'lib', 'binding'))

assert(iastnativemethods, 'Undefined');
assert(iastnativemethods.newTaintedString, 'Undefined');
assert(iastnativemethods.isTainted, 'Undefined');
