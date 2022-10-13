/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/

const path = require('path');
const { TaintedUtils } = require('./util');
const assert = require('assert');

describe('Substring method', function () {
    let id = '1';

    afterEach(function () {
        TaintedUtils.removeTransaction(id);
    });

    it('Wrong arguments', function () {
        assert.throws( function () { TaintedUtils.substring(id) ;}, Error);
    })

    it('String not tainted', function () {
        let op1 = 'hello';
        let start = 1;
        let end = op1.length;

        let res = op1.substring(start, end);
        let ret = TaintedUtils.substring(id, op1, res, start, end);
        assert.equal(res, ret, "Unexpected vale");
        assert.equal(false, TaintedUtils.isTainted(id, ret), "Unexpected value");
    })

    it('Range not tainted', function () {
        let op1 = 'hello';
        let op2 = ' world';
        let start = op1.length;

        op1 = TaintedUtils.newTaintedString(id, op1, 'param', 'REQUEST');

        let res = TaintedUtils.concat(id, op1 + op2, op1, op2);

        let ret = res.substring(start, start);

        ret = TaintedUtils.substring(id, op1, ret, start, res.length);
        assert.equal(false, TaintedUtils.isTainted(id, ret), "Unexpected value");
    })


    it('Check result', function () {
        let op1 = 'hello';
        let start = 1;
        let end = op1.length;

        op1 = TaintedUtils.newTaintedString(id, op1, 'param', 'REQUEST');
        assert.strictEqual(op1, 'hello', "Unexpected value");
        assert.equal(true, TaintedUtils.isTainted(id, op1), "Unexpected value");

        let res = op1.substring(start, end);
        let ret = TaintedUtils.substring(id, op1, res, start, end);
        assert.equal(res, ret, "Unexpected vale");
        assert.equal(true, TaintedUtils.isTainted(id, ret), "Unexpected value");

    })

    it('Check ranges', function () {
        let op1 = 'hello';
        let start = 1;
        let end = op1.length;

        let res = op1.substr(1, end);

        let expected = [ 
            { start: 0, end: res.length, iinfo : 
                { parameterName: 'param',
                  parameterValue: 'hello',
                  type: 'REQUEST'
                }
            }
        ];

        op1 = TaintedUtils.newTaintedString(id, op1, 'param', 'REQUEST');
        assert.strictEqual(op1, 'hello', "Unexpected value");
        assert.equal(true, TaintedUtils.isTainted(id, op1), "Unexpected value");

        let ret = TaintedUtils.substring(id, op1, res, start, end);

        assert.equal(res, ret, "Unexpected vale");
        assert.equal(true, TaintedUtils.isTainted(id, ret), "Unexpected value");
        assert.deepEqual(expected, TaintedUtils.getRanges(id, ret), "Unexpected ranges");

    })

    it('Check ranges zero length substring', function () {
        let op1 = 'hello';
        let start = op1.length;
        let end = op1.length;

        let res = op1.substr(start, end);

        op1 = TaintedUtils.newTaintedString(id, op1, 'param', 'REQUEST');
        assert.strictEqual(op1, 'hello', "Unexpected value");
        assert.equal(true, TaintedUtils.isTainted(id, op1), "Unexpected value");

        let ret = TaintedUtils.substring(id, op1, res, start, end);

        assert.equal(res, ret, "Unexpected vale");
        assert.equal(false, TaintedUtils.isTainted(id, ret), "Unexpected value");

    })
})
