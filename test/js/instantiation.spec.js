const path = require('path');
const assert = require('assert');
const { TaintedUtils } = require('./util');

describe('Addon loading', function () {
    it ('Check native methods', function () {
        assert(TaintedUtils, 'Undefined module');
        assert(TaintedUtils.newTaintedString, 'Undefined');
        assert(TaintedUtils.isTainted, 'Undefined');
        assert(TaintedUtils.concat, 'Undefined');
    })
})
