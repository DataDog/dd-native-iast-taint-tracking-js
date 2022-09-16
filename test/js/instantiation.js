const path = require('path');
const assert = require('assert');
const iastnativemethods = require(path.join(__dirname, '..', '..', 'lib', 'binding'))

describe('Addon loading', function () {
    it ('Check native methods', function () {
        assert(iastnativemethods, 'Undefined module');
        assert(iastnativemethods.newTaintedString, 'Undefined');
        assert(iastnativemethods.isTainted, 'Undefined');
        assert(iastnativemethods.concat, 'Undefined');
    })
})
