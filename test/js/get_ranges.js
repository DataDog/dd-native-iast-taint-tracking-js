const path = require('path');
const iastnativemethods = require(path.join(__dirname, '..', '..', 'lib', 'binding'));
const assert = require('assert');

describe('Ranges', function () {
    let value = 'test';
    let id = '1';

    afterEach(function () {
        iastnativemethods.removeTransaction(id);
    });

    it('Get Ranges', function () {
        let expected = [ { start: 0, end: value.length, iinfo : { parameterName: 'param', parameterValue: 'test', type: 'REQUEST' }} ];
        iastnativemethods.newTaintedString(id, value, 'param', 'REQUEST');

        let ranges = iastnativemethods.getRanges(id, value);
        assert.deepEqual(ranges, expected, "Ranges expected to be equal");
    })

    it('Wrong number of arguments', function () {
        assert.throws(function () { iastnativemethods.getRanges(id); }, Error);
    })

    it('Get ranges from non tainted string', function () {
        let expected;
        let nonTainted = 'value';

        iastnativemethods.newTaintedString(id, value, 'param', 'REQUEST');
        let ranges = iastnativemethods.getRanges(id, nonTainted);
        assert.equal(ranges, undefined, "Ranges expected to be equal");
    })
})
