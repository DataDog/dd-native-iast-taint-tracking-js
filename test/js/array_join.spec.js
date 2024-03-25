/**
 * Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
 * This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
 **/
const { TaintedUtils, taintFormattedString, formatTaintedValue } = require('./util')
const assert = require('assert')

describe('Array join operator', function () {
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
      testArray: ['佫:+-𝒳-+:佫', ':+-😂😂😂-+:', 'abc', '𝒳:+-佫佫佫-+:𝒳'],
      testSeparator: undefined,
      joinResult: '佫:+-𝒳-+:佫,:+-😂😂😂-+:,abc,𝒳:+-佫佫佫-+:𝒳'
    },
    {
      testArray: ['佫:+-𝒳-+:佫', ':+-😂😂😂-+:', 'abc', '𝒳:+-佫佫佫-+:𝒳'],
      testSeparator: '###',
      joinResult: '佫:+-𝒳-+:佫###:+-😂😂😂-+:###abc###𝒳:+-佫佫佫-+:𝒳'
    },
    {
      testArray: ['佫:+-𝒳-+:佫', ':+-😂😂😂-+:', 'abc', '𝒳:+-佫佫佫-+:𝒳'],
      testSeparator: 123,
      joinResult: '佫:+-𝒳-+:佫123:+-😂😂😂-+:123abc123𝒳:+-佫佫佫-+:𝒳'
    },
    {
      testArray: ['佫:+-𝒳-+:佫', ':+-😂😂😂-+:', 'abc', '𝒳:+-佫佫佫-+:𝒳'],
      testSeparator: ':+-###-+:',
      joinResult: '佫:+-𝒳-+:佫:+-###-+::+-😂😂😂-+::+-###-+:abc:+-###-+:𝒳:+-佫佫佫-+:𝒳'
    },
    {
      testArray: ['佫:+-𝒳-+:佫', ':+-😂😂😂-+:', 'abc', '𝒳:+-佫佫佫-+:𝒳'],
      testSeparator: '佫𝒳😂',
      joinResult: '佫:+-𝒳-+:佫佫𝒳😂:+-😂😂😂-+:佫𝒳😂abc佫𝒳😂𝒳:+-佫佫佫-+:𝒳'
    },
    {
      testArray: ['佫:+-𝒳-+:佫', ':+-😂😂😂-+:', 'abc', '𝒳:+-佫佫佫-+:𝒳'],
      testSeparator: '佫:+-𝒳-+:😂',
      joinResult: '佫:+-𝒳-+:佫佫:+-𝒳-+:😂:+-😂😂😂-+:佫:+-𝒳-+:😂abc佫:+-𝒳-+:😂𝒳:+-佫佫佫-+:𝒳'
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
    },
    {
      testArray: [{ a: 1 }, ':+-abc-+:', 666],
      testSeparator: undefined,
      joinResult: '[object Object],:+-abc-+:,666'
    },
    {
      testArray: [{ a: 1 }, ':+-abc-+:', 666],
      testSeparator: ':+-o-+:',
      joinResult: '[object Object]:+-o-+::+-abc-+::+-o-+:666'
    },
    {
      testArray: [{ a: 1 }, ':+-abc-+:', 666],
      testSeparator: 'AB:+-CD-+:EF',
      joinResult: '[object Object]AB:+-CD-+:EF:+-abc-+:AB:+-CD-+:EF666'
    },
    {
      testArray: [undefined, ':+-abc-+:', 666],
      testSeparator: undefined,
      joinResult: ',:+-abc-+:,666'
    },
    {
      testArray: [undefined, ':+-abc-+:', 666],
      testSeparator: ':+-o-+:',
      joinResult: ':+-o-+::+-abc-+::+-o-+:666'
    },
    {
      testArray: [undefined, ':+-abc-+:', 666],
      testSeparator: 'AB:+-CD-+:EF',
      joinResult: 'AB:+-CD-+:EF:+-abc-+:AB:+-CD-+:EF666'
    },
    {
      testArray: [null, ':+-abc-+:', 666],
      testSeparator: undefined,
      joinResult: ',:+-abc-+:,666'
    },
    {
      testArray: [null, ':+-abc-+:', 666],
      testSeparator: ':+-o-+:',
      joinResult: ':+-o-+::+-abc-+::+-o-+:666'
    },
    {
      testArray: [null, ':+-abc-+:', 666],
      testSeparator: 'AB:+-CD-+:EF',
      joinResult: 'AB:+-CD-+:EF:+-abc-+:AB:+-CD-+:EF666'
    },
    {
      testArray: [[':+-foo-+:', 'bar'], ':+-abc-+:', 666],
      testSeparator: ':+-o-+:',
      joinResult: ':+-foo-+:,bar:+-o-+::+-abc-+::+-o-+:666'
    },
    {
      testArray: [null, undefined, { a: 1 }, [':+-foo-+:', '佫:+-𝒳-+:佫', 'bar'], ':+-abc-+:', 666],
      testSeparator: ':+-###-+:',
      joinResult: ':+-###-+::+-###-+:[object Object]:+-###-+::+-foo-+:,佫:+-𝒳-+:佫,bar:+-###-+::+-abc-+::+-###-+:666'
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
    
    it('With empty array', function () {
      TaintedUtils.newTaintedString(id, 'tainted', 'param', 'request')
      assert.doesNotThrow(function () {
        TaintedUtils.arrayJoin(id, '', [])
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
