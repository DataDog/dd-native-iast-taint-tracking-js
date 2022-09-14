const path = require('path');
const iastnativemethods = require(path.join(__dirname, '..', '..', 'lib', 'binding'));
const assert = require('assert');

describe('Taint strings', function () {
    let value = 'test';
    let id = '1';

    afterEach(function () {
        iastnativemethods.endTransaction(id);
    });

    it('Taint new string', function () {
        let ret;

        ret = iastnativemethods.newTaintedString(id, value, 'param', 'REQUEST');
        assert.strictEqual(ret, 'test', "Unexpected value");
    })

    it('Wrong number of args', function () {
        let ret;

        assert.throws( function () { iastnativemethods.newTaintedString(id, value);}, Error);
        assert.throws( function () { iastnativemethods.isTainted(id);}, Error);
    })

    it('Check untainted string', function () {
        let ret;

        ret = iastnativemethods.isTainted(id, value);
        assert.strictEqual(ret, false, "Unexpected value");
    })

    it('Check tainted string', function () {
        let ret;

        iastnativemethods.newTaintedString(id, value, 'param', 'REQUEST');
        ret = iastnativemethods.isTainted(id, value);
        assert.strictEqual(ret, true, "Unexpected value");
    })

    it('Check tainted string with unknown id', function () {
        let ret;
        let wrongId = '2';

        iastnativemethods.newTaintedString(id, value, 'param', 'REQUEST');
        ret = iastnativemethods.isTainted(wrongId, value);
        assert.strictEqual(ret, false, "Unexpected value");
    })

    it('Taint number', function () {
        let ret;
        let value = 123;

        ret = iastnativemethods.newTaintedString(id, value, 'param', 'REQUEST');
        assert.strictEqual(ret, 123, "Unexpected value");
        ret = iastnativemethods.isTainted(id, value);
        assert.strictEqual(ret, false, "Unexpected value");
    })

    it('Taint object', function () {
        let ret;
        let value = {};

        ret = iastnativemethods.newTaintedString(id, value, 'param', 'REQUEST');
        assert.strictEqual(ret, value, "Unexpected value");
        ret = iastnativemethods.isTainted(id, value);
        assert.strictEqual(ret, false, "Unexpected value");
    })

    it('Taint undefined', function () {
        let ret;
        let value;

        ret = iastnativemethods.newTaintedString(id, value, 'param', 'REQUEST');
        assert.strictEqual(ret, undefined, "Unexpected value");
        ret = iastnativemethods.isTainted(id, value);
        assert.strictEqual(ret, false, "Unexpected value");
    })

    it('Taint null', function () {
        let ret;
        let value = null;

        ret = iastnativemethods.newTaintedString(id, value, 'param', 'REQUEST');
        assert.strictEqual(ret, null, "Unexpected value");
        ret = iastnativemethods.isTainted(id, value);
        assert.strictEqual(ret, false, "Unexpected value");
    })

    it('Max values', function () {
        let ret;
        let values = new Array(16384).fill("value");

        values.forEach((val,index,array) => {
            ret = iastnativemethods.newTaintedString(id, val, 'param', 'REQUEST');
            assert.strictEqual(ret, "value", "Unexpected value");
            ret = iastnativemethods.isTainted(id, val);
            assert.strictEqual(ret, true, "Unexpected value");
        });
    })

    it('Beyond Max values', function () {
        let ret;
        let values = new Array(16384);
        for (let i = 0; i < values.length; i++) {
            values[i] = i.toString();
            ret = iastnativemethods.newTaintedString(id, values[i], 'param', 'REQUEST');
            assert.strictEqual(ret, values[i], "Unexpected value");
            ret = iastnativemethods.isTainted(id, values[i]);
            assert.strictEqual(ret, true, "Unexpected value");
        }


        //element 16385
        let beyondLimit = "beyond";
        ret = iastnativemethods.newTaintedString(id, beyondLimit, 'param', 'REQUEST');
        assert.strictEqual(ret, "beyond", "Unexpected value");
        ret = iastnativemethods.isTainted(id, beyondLimit);
        assert.strictEqual(ret, false, "Unexpected value");

    })
})

