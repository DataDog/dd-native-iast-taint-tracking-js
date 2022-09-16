const path = require('path');
const iastnativemethods = require(path.join(__dirname, '..', '..', 'lib', 'binding'));
const assert = require('assert');

describe('Plus operator', function () {
    let id = '1';

    afterEach(function () {
        iastnativemethods.removeTransaction(id);
    });

    it('Wrong arguments', function () {


        assert.throws( function () { iastnativemethods.concat(id) ;}, Error);
    })

    it('Check result', function () {
        let ret;
        let op1 = 'hello';
        let op2 = ' world'; 

        op1 = iastnativemethods.newTaintedString(id, op1, 'param', 'REQUEST');
        assert.strictEqual(op1, 'hello', "Unexpected value");
        assert.equal(true, iastnativemethods.isTainted(id, op1), "Unexpected value");

        op2 = iastnativemethods.newTaintedString(id, op2, 'param', 'REQUEST');
        assert.strictEqual(op2, ' world', "Unexpected value");
        assert.equal(true, iastnativemethods.isTainted(id, op2), "Unexpected value");

        let res = op1 + op2;
        ret = iastnativemethods.concat(id, res, op1, op2);
        assert.equal(res, ret, "Unexpected vale");
        assert.equal(true, iastnativemethods.isTainted(id, ret), "Unexpected value");

    })

    it('Check ranges', function () {
        let ret;
        let op1 = 'hello';
        let op2 = ' world'; 
        let res = op1 + op2;

        let expected = [ 
            { start: 0, end: op1.length, iinfo : 
                { parameterName: 'param',
                  parameterValue: 'hello',
                  type: 'REQUEST'
                }
            },
            { start: op1.length, end: res.length, iinfo : 
                { parameterName: 'param',
                  parameterValue: ' world',
                  type: 'REQUEST'
                }
            },
        ];

        op1 = iastnativemethods.newTaintedString(id, op1, 'param', 'REQUEST');
        assert.strictEqual(op1, 'hello', "Unexpected value");
        assert.equal(true, iastnativemethods.isTainted(id, op1), "Unexpected value");

        op2 = iastnativemethods.newTaintedString(id, op2, 'param', 'REQUEST');
        assert.strictEqual(op2, ' world', "Unexpected value");
        assert.equal(true, iastnativemethods.isTainted(id, op2), "Unexpected value");

        ret = iastnativemethods.concat(id, res, op1, op2);
        assert.equal(res, ret, "Unexpected vale");
        assert.equal(true, iastnativemethods.isTainted(id, ret), "Unexpected value");
        
        assert.deepEqual(expected, iastnativemethods.getRanges(id, ret), "Unexpected ranges");

    })

    it('Check ranges just op1 tainted', function () {
        let ret;
        let op1 = 'hello';
        let op2 = ' world'; 
        let res = op1 + op2;

        let expected = [ 
            { start: 0, end: op1.length, iinfo : 
                { parameterName: 'param',
                  parameterValue: 'hello',
                  type: 'REQUEST'
                }
            }
        ];

        op1 = iastnativemethods.newTaintedString(id, op1, 'param', 'REQUEST');
        assert.strictEqual(op1, 'hello', "Unexpected value");
        assert.equal(true, iastnativemethods.isTainted(id, op1), "Unexpected value");

        ret = iastnativemethods.concat(id, res, op1, op2);
        assert.equal(res, ret, "Unexpected vale");
        assert.equal(true, iastnativemethods.isTainted(id, ret), "Unexpected value");

        assert.deepEqual(expected, iastnativemethods.getRanges(id, ret));
    })

    it('Check ranges just op2 tainted', function () {
        let ret;
        let op1 = 'hello';
        let op2 = ' world'; 
        let res = op1 + op2;

        let expected = [ 
            { start: op1.length, end: res.length, iinfo : 
                { parameterName: 'param',
                  parameterValue: ' world',
                  type: 'REQUEST'
                }
            }
        ];

        op2 = iastnativemethods.newTaintedString(id, op2, 'param', 'REQUEST');
        assert.strictEqual(op2, ' world', "Unexpected value");
        assert.equal(true, iastnativemethods.isTainted(id, op2), "Unexpected value");

        ret = iastnativemethods.concat(id, res, op1, op2);
        assert.equal(res, ret, "Unexpected vale");
        assert.equal(true, iastnativemethods.isTainted(id, ret), "Unexpected value");

        assert.deepEqual(expected, iastnativemethods.getRanges(id, ret));
    })
})
