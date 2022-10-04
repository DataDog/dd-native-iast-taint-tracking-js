const path = require('path');
const TaintedUtils = require('../../index');
const assert = require('assert');

describe('Ranges', function () {
    let value = 'test';
    let param = 'p' + 'aram';
    let id = '666';

    afterEach(function () {
        TaintedUtils.removeTransaction(id);
    });

    it('Get Ranges', function () {
        let expected = [ { start: 0, end: value.length, iinfo : { parameterName: 'param', parameterValue: 'test', type: 'REQUEST' }} ];
        TaintedUtils.newTaintedString(id, value, param, 'REQUEST');

        let ranges = TaintedUtils.getRanges(id, value);
        assert.deepEqual(ranges, expected, "Ranges expected to be equal");
    })

    it('Wrong number of arguments', function () {
        assert.throws(function () { TaintedUtils.getRanges(id); }, Error);
    })

    it('Get ranges from non tainted string', function () {
        let expected;
        let nonTainted = 'value';

        TaintedUtils.newTaintedString(id, value, param, 'REQUEST');
        let ranges = TaintedUtils.getRanges(id, nonTainted);
        assert.equal(ranges, undefined, "Ranges expected to be equal");
    })
})
