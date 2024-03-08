/**
 * Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
 * This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
 **/
const { TaintedUtils, taintFormattedString, formatTaintedValue } = require('./util')
const assert = require('assert')

describe('String case operator', function () {
  const id = TaintedUtils.createTransaction('1')

  const rangesTestCases = [
    {
      testArray: [':+-a-+:'],
      testSeparator: undefined,
      joinResult: ':+-a-+:'
    },
    {
      testArray: [':+-a-+:'],
      testSeparator: '###',
      joinResult: ':+-a-+:'
    },
    {
      testArray: [':+-a-+:'],
      testSeparator: 123,
      joinResult: ':+-a-+:'
    },
    {
      testArray: [':+-a-+:'],
      testSeparator: ':+-###-+:',
      joinResult: ':+-a-+:'
    },
    {
      testArray: [':+-abc-+:', ':+-def-+:'],
      testSeparator: undefined,
      joinResult: ':+-abc-+:,:+-def-+:'
    },
    {
      testArray: [':+-abc-+:', ':+-def-+:'],
      testSeparator: '###',
      joinResult: ':+-abc-+:###:+-def-+:'
    },
    {
      testArray: [':+-abc-+:', ':+-def-+:'],
      testSeparator: 123,
      joinResult: ':+-abc-+:123:+-def-+:'
    },
    {
      testArray: [':+-abc-+:', 'def'],
      testSeparator: undefined,
      joinResult: ':+-abc-+:,def'
    },
    {
      testArray: [':+-abc-+:', 'def'],
      testSeparator: '###',
      joinResult: ':+-abc-+:###def'
    },
    {
      testArray: [':+-abc-+:', 'def'],
      testSeparator: 123,
      joinResult: ':+-abc-+:123def'
    },
    {
      testArray: ['abc', 'def'],
      testSeparator: ':+-###-+:',
      joinResult: 'abc:+-###-+:def'
    },
    {
      testArray: [':+-abc-+:', 'def', 'gh:+-ij-+:kl'],
      testSeparator: ':+-###-+:',
      joinResult: ':+-abc-+::+-###-+:def:+-###-+:gh:+-ij-+:kl'
    },
    {
      testArray: [':+-abc-+:', 'def', 'gh:+-ij-+:kl'],
      testSeparator: 'AB:+-CD-+:EF',
      joinResult: ':+-abc-+:AB:+-CD-+:EFdefAB:+-CD-+:EFgh:+-ij-+:kl'
    },
    {
      testArray: [':+-abc-+:', 'def', 'gh:+-ij-+:kl'],
      testSeparator: ':+-o-+:',
      joinResult: ':+-abc-+::+-o-+:def:+-o-+:gh:+-ij-+:kl'
    }
  ]

  function testArrayJoinResult (arrayJoinFn, taintedArrayJoinFn) {
    let testString = 'sit amet'
    let testSeparator = '-'
    const testArray = ['lorem', 'ipsum', 1234]

    testString = TaintedUtils.newTaintedString(id, testString, 'PARAM_NAME', 'PARAM_TYPE')
    testSeparator = TaintedUtils.newTaintedString(id, testSeparator, 'PARAM_NAME', 'PARAM_TYPE')

    assert.strictEqual(testString, 'sit amet', 'Unexpected value')
    assert.equal(true, TaintedUtils.isTainted(id, testString), 'Unexpected value')
    assert.strictEqual(testSeparator, '-', 'Unexpected value')
    assert.equal(true, TaintedUtils.isTainted(id, testSeparator), 'Unexpected value')

    testArray.push(testString)

    const res = arrayJoinFn.call(testArray, testSeparator)
    const ret = taintedArrayJoinFn(id, res, testArray, testSeparator)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(true, TaintedUtils.isTainted(id, ret), 'Unexpected value')
  }

  function testArrayJoinNoTaintedResult (arrayJoinFn, taintedArrayJoinFn) {
    const testArray = ['lorem', 'ipsum', 1234]
    const testSeparator = '-'
    const res = arrayJoinFn.call(testArray, testSeparator)
    const ret = taintedArrayJoinFn(id, res, testArray, testSeparator)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(false, TaintedUtils.isTainted(id, ret), 'Unexpected value')
  }

  function testArrayJoinCheckRanges (
    arrayJoinFn,
    taintedArrayJoinFn,
    formattedTestArray,
    formattedTestSeparator,
    expectedResult
  ) {
    const testArray = formattedTestArray.map((formattedTestString) => taintFormattedString(id, formattedTestString))
    const testSeparator = taintFormattedString(id, formattedTestSeparator)
    const res = arrayJoinFn.call(testArray, testSeparator)
    const ret = taintedArrayJoinFn(id, res, testArray, testSeparator)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(TaintedUtils.isTainted(id, ret), true, 'Array join returned value not tainted')

    const formattedResult = formatTaintedValue(id, ret)
    assert.equal(formattedResult, expectedResult, 'Unexpected ranges')
  }

  afterEach(function () {
    TaintedUtils.removeTransaction(id)
  })

  describe('Array.join', function () {
    it('Wrong arguments', function () {
      assert.throws(function () {
        TaintedUtils.arrayJoin(id)
      }, Error)
    })

    it('Without separator', function () {
      assert.doesNotThrow(function () {
        TaintedUtils.arrayJoin(id, 'result', ['test', 'array'])
      }, Error)
    })

    it('With separator', function () {
      assert.doesNotThrow(function () {
        TaintedUtils.arrayJoin(id, 'result', ['test', 'array'], 'separator')
      }, Error)
    })

    it('Check result', function () {
      testArrayJoinResult(Array.prototype.join, TaintedUtils.arrayJoin)
    })

    it('Check result from not tainted value', function () {
      testArrayJoinNoTaintedResult(Array.prototype.join, TaintedUtils.arrayJoin)
    })

    it('Secure marks are inherited', () => {
      let op1 = 'HELLO'
      op1 = TaintedUtils.newTaintedString(id, op1, 'param1', 'REQUEST')
      op1 = TaintedUtils.addSecureMarksToTaintedString(id, op1, 0b0110)

      let op2 = 'WORLD'
      op2 = TaintedUtils.newTaintedString(id, op2, 'param2', 'REQUEST')
      op2 = TaintedUtils.addSecureMarksToTaintedString(id, op2, 0b0100)

      let separator = ' --- '
      separator = TaintedUtils.newTaintedString(id, separator, 'separator', 'REQUEST')
      separator = TaintedUtils.addSecureMarksToTaintedString(id, separator, 0b0101)

      const arrayTest = [op1, op2]

      let result = arrayTest.join(separator)
      result = TaintedUtils.arrayJoin(id, result, arrayTest, separator)

      const ranges = TaintedUtils.getRanges(id, result)
      assert.equal(ranges.length, 3)
      assert.equal(ranges[0].secureMarks, 0b0110)
      assert.equal(ranges[1].secureMarks, 0b0101)
      assert.equal(ranges[2].secureMarks, 0b0100)
    })

    describe('Check ranges', function () {
      rangesTestCases.forEach(({ testArray, testSeparator, joinResult }) => {
        it(`Test ${joinResult}`, () => {
          testArrayJoinCheckRanges(Array.prototype.join, TaintedUtils.arrayJoin, testArray, testSeparator, joinResult)
        })
      })
    })
  })
})
