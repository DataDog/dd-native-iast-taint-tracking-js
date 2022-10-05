const path = require('path');
const { TaintedUtils } = require('./util');
const assert = require('assert');

describe('Taint strings', function () {
    let value = 'test';
    let id = '1';

    afterEach(function () {
        TaintedUtils.removeTransaction(id);
    });

    it('Taint new string', function () {
        let ret;

        ret = TaintedUtils.newTaintedString(id, value, 'param', 'REQUEST');
        assert.strictEqual(ret, 'test', "Unexpected value");
    })

    it('Wrong number of args', function () {
        let ret;

        assert.throws( function () { TaintedUtils.newTaintedString(id, value);}, Error);
        assert.throws( function () { TaintedUtils.isTainted(id);}, Error);
    })

    it('Check untainted string', function () {
        let ret;

        ret = TaintedUtils.isTainted(id, value);
        assert.strictEqual(ret, false, "Unexpected value");
    })

    it('Check tainted string', function () {
        let ret;

        TaintedUtils.newTaintedString(id, value, 'param', 'REQUEST');
        ret = TaintedUtils.isTainted(id, value);
        assert.strictEqual(ret, true, "Unexpected value");
    })

    it('Check tainted string with unknown id', function () {
        let ret;
        let wrongId = '2';

        TaintedUtils.newTaintedString(id, value, 'param', 'REQUEST');
        ret = TaintedUtils.isTainted(wrongId, value);
        assert.strictEqual(ret, false, "Unexpected value");
    })

    it('Taint number', function () {
        let ret;
        let value = 123;

        ret = TaintedUtils.newTaintedString(id, value, 'param', 'REQUEST');
        assert.strictEqual(ret, 123, "Unexpected value");
        ret = TaintedUtils.isTainted(id, value);
        assert.strictEqual(ret, false, "Unexpected value");
    })

    it('Taint object', function () {
        let ret;
        let value = {};

        ret = TaintedUtils.newTaintedString(id, value, 'param', 'REQUEST');
        assert.strictEqual(ret, value, "Unexpected value");
        ret = TaintedUtils.isTainted(id, value);
        assert.strictEqual(ret, false, "Unexpected value");
    })

    it('Taint undefined', function () {
        let ret;
        let value;

        ret = TaintedUtils.newTaintedString(id, value, 'param', 'REQUEST');
        assert.strictEqual(ret, undefined, "Unexpected value");
        ret = TaintedUtils.isTainted(id, value);
        assert.strictEqual(ret, false, "Unexpected value");
    })

    it('Taint null', function () {
        let ret;
        let value = null;

        ret = TaintedUtils.newTaintedString(id, value, 'param', 'REQUEST');
        assert.strictEqual(ret, null, "Unexpected value");
        ret = TaintedUtils.isTainted(id, value);
        assert.strictEqual(ret, false, "Unexpected value");
    })

    it('Max values', function () {
        let ret;
        let values = new Array(16384).fill("value");

        values.forEach((val,index,array) => {
            ret = TaintedUtils.newTaintedString(id, val, 'param', 'REQUEST');
            assert.strictEqual(ret, "value", "Unexpected value");
            ret = TaintedUtils.isTainted(id, val);
            assert.strictEqual(ret, true, "Unexpected value");
        });
    })

    it('Beyond Max values', function () {
        let ret;
        //let id = '1';
        let values = new Array(16384);
        for (let i = 0; i < values.length; i++) {
            values[i] = i.toString();
            ret = TaintedUtils.newTaintedString(id, values[i], 'param', 'REQUEST');
            assert.strictEqual(ret, values[i], "Unexpected value");
            ret = TaintedUtils.isTainted(id, ret);
            assert.strictEqual(ret, true, "Unexpected value");
        }


        //element 16385
        let beyondLimit = "beyond";
        ret = TaintedUtils.newTaintedString(id, beyondLimit, 'param', 'REQUEST');
        assert.strictEqual(ret, "beyond", "Unexpected value");
        ret = TaintedUtils.isTainted(id, beyondLimit);
        assert.strictEqual(ret, false, "Unexpected value");

    })
})

