/**
 * Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
 * This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
 **/
const { TaintedUtils, taintFormattedString, formatTaintedValue } = require('./util')
const assert = require('assert')

describe('Plus operator', function () {
  const id = '1'

  afterEach(function () {
    TaintedUtils.removeTransaction(id)
  })

  it('Wrong arguments', function () {
    assert.throws(function () {
      TaintedUtils.concat(id)
    }, Error)
  })

  it('Check result', function () {
    let op1 = 'hello'
    let op2 = ' world'

    op1 = TaintedUtils.newTaintedString(id, op1, 'param', 'REQUEST')
    assert.strictEqual(op1, 'hello', 'Unexpected value')
    assert.equal(true, TaintedUtils.isTainted(id, op1), 'Unexpected value')

    op2 = TaintedUtils.newTaintedString(id, op2, 'param', 'REQUEST')
    assert.strictEqual(op2, ' world', 'Unexpected value')
    assert.equal(true, TaintedUtils.isTainted(id, op2), 'Unexpected value')

    const res = op1 + op2
    const ret = TaintedUtils.concat(id, res, op1, op2)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(true, TaintedUtils.isTainted(id, ret), 'Unexpected value')
  })

  it('Check ranges', function () {
    let op1 = 'hello'
    let op2 = ' world'
    const res = op1 + op2

    const expected = [
      {
        start: 0,
        end: op1.length,
        iinfo:
          {
            parameterName: 'param',
            parameterValue: 'hello',
            type: 'REQUEST'
          }
      },
      {
        start: op1.length,
        end: res.length,
        iinfo:
          {
            parameterName: 'param',
            parameterValue: ' world',
            type: 'REQUEST'
          }
      }
    ]

    op1 = TaintedUtils.newTaintedString(id, op1, 'param', 'REQUEST')
    assert.strictEqual(op1, 'hello', 'Unexpected value')
    assert.equal(true, TaintedUtils.isTainted(id, op1), 'Unexpected value')

    op2 = TaintedUtils.newTaintedString(id, op2, 'param', 'REQUEST')
    assert.strictEqual(op2, ' world', 'Unexpected value')
    assert.equal(true, TaintedUtils.isTainted(id, op2), 'Unexpected value')

    const ret = TaintedUtils.concat(id, res, op1, op2)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(true, TaintedUtils.isTainted(id, ret), 'Unexpected value')

    assert.deepEqual(expected, TaintedUtils.getRanges(id, ret), 'Unexpected ranges')
  })

  it('Check ranges just op1 tainted', function () {
    let op1 = 'hello'
    const op2 = ' world'
    const res = op1 + op2

    const expected = [
      {
        start: 0,
        end: op1.length,
        iinfo:
          {
            parameterName: 'param',
            parameterValue: 'hello',
            type: 'REQUEST'
          }
      }
    ]

    op1 = TaintedUtils.newTaintedString(id, op1, 'param', 'REQUEST')
    assert.strictEqual(op1, 'hello', 'Unexpected value')
    assert.equal(true, TaintedUtils.isTainted(id, op1), 'Unexpected value')

    const ret = TaintedUtils.concat(id, res, op1, op2)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(true, TaintedUtils.isTainted(id, ret), 'Unexpected value')

    assert.deepEqual(expected, TaintedUtils.getRanges(id, ret))
  })

  it('Check ranges just op2 tainted', function () {
    const op1 = 'hello'
    let op2 = ' world'
    const res = op1 + op2

    const expected = [
      {
        start: op1.length,
        end: res.length,
        iinfo:
          {
            parameterName: 'param',
            parameterValue: ' world',
            type: 'REQUEST'
          }
      }
    ]

    op2 = TaintedUtils.newTaintedString(id, op2, 'param', 'REQUEST')
    assert.strictEqual(op2, ' world', 'Unexpected value')
    assert.equal(true, TaintedUtils.isTainted(id, op2), 'Unexpected value')

    const ret = TaintedUtils.concat(id, res, op1, op2)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(true, TaintedUtils.isTainted(id, ret), 'Unexpected value')

    assert.deepEqual(expected, TaintedUtils.getRanges(id, ret))
  })
})

describe('concat method', () => {
  const id = '1'

  afterEach(function () {
    TaintedUtils.removeTransaction(id)
  })

  const rangesTestCases = [
    {
      testStrings: [':+-A-+:', 'B', 'C'],
      result: ':+-A-+:BC'
    },
    {
      testStrings: ['A', ':+-B-+:', 'C'],
      result: 'A:+-B-+:C'
    },
    {
      testStrings: ['A', 'B', ':+-C-+:'],
      result: 'AB:+-C-+:'
    },
    {
      testStrings: ['A', '�', ':+-C-+:'],
      result: 'A�:+-C-+:'
    },
    {
      testStrings: ['A', null, ':+-C-+:'],
      result: 'Anull:+-C-+:'
    },
    {
      testStrings: ['A', undefined, ':+-C-+:'],
      result: 'Aundefined:+-C-+:'
    },
    {
      testStrings: ['A', 'B', 'C'],
      result: 'ABC',
      tainted: false
    },
    {
      testStrings: [':+-A-+:', 'B', ':+-C-+:'],
      result: ':+-A-+:B:+-C-+:'
    },
    {
      testStrings: [':+-A-+:', ':+-B-+:', 'C'],
      result: ':+-A-+::+-B-+:C'
    },
    {
      testStrings: [':+-A-+: BC :+-D-+:', ':+-E-+:', 'F'],
      result: ':+-A-+: BC :+-D-+::+-E-+:F'
    },
    {
      testStrings: [':+-A-+: ���', ':+-B-+:', 'C'],
      result: ':+-A-+: ���:+-B-+:C'
    },
    {
      testStrings: [':+-A-+:', 1, 'C'],
      result: ':+-A-+:1C'
    },
    {
      testStrings: ['A', {}, ':+-C-+:'],
      result: 'A[object Object]:+-C-+:'
    }
  ]

  function testConcatCheckRanges (formattedTestStrings, expectedResult, resultTainted) {
    const [testString, ...rest] = formattedTestStrings
      .map(formattedTestString => taintFormattedString(id, formattedTestString))
    const res = String.prototype.concat.call(testString, ...rest)

    const ret = TaintedUtils.concat(id, res, testString, ...rest)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(TaintedUtils.isTainted(id, ret), resultTainted,
      `Concat returned value ${resultTainted ? 'not tainted' : 'tainted'}`)

    const formattedResult = resultTainted ? formatTaintedValue(id, ret) : ret
    assert.equal(formattedResult, expectedResult, 'Unexpected ranges')
  }

  describe('Check ranges', function () {
    rangesTestCases.forEach(({ testStrings, result, tainted }) => {
      it(`Test ${testStrings}`, () => {
        const resultTainted = tainted === undefined ? true : tainted
        testConcatCheckRanges(testStrings, result, resultTainted)
      })
    })
  })
})
