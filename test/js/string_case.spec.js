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
      testString: ':+-abcdef-+:',
      toLowerResult: ':+-abcdef-+:',
      toUpperResult: ':+-ABCDEF-+:'
    },
    {
      testString: ':+-ABCDEF-+:',
      toLowerResult: ':+-abcdef-+:',
      toUpperResult: ':+-ABCDEF-+:'
    },
    {
      testString: ':+-ABCdef-+:',
      toLowerResult: ':+-abcdef-+:',
      toUpperResult: ':+-ABCDEF-+:'
    },
    {
      testString: ':+-ABC-+::+-def-+:',
      toLowerResult: ':+-abc-+::+-def-+:',
      toUpperResult: ':+-ABC-+::+-DEF-+:'
    },
    {
      testString: ':+-ABC-+:dEf:+-ghi-+:',
      toLowerResult: ':+-abc-+:def:+-ghi-+:',
      toUpperResult: ':+-ABC-+:DEF:+-GHI-+:'
    },
    {
      testString: 'ABC:+-def-+:',
      toLowerResult: 'abc:+-def-+:',
      toUpperResult: 'ABC:+-DEF-+:'
    },
    {
      testString: ':+-ABC-+:def',
      toLowerResult: ':+-abc-+:def',
      toUpperResult: ':+-ABC-+:DEF'
    },
    {
      testString: ':+-佫😂😂😂𝒳-+:',
      toLowerResult: ':+-佫😂😂😂𝒳-+:',
      toUpperResult: ':+-佫😂😂😂𝒳-+:'
    },
    {
      testString: ':+-佫-+::+-😂😂😂-+::+-𝒳-+:',
      toLowerResult: ':+-佫-+::+-😂😂😂-+::+-𝒳-+:',
      toUpperResult: ':+-佫-+::+-😂😂😂-+::+-𝒳-+:'
    },
    {
      testString: 'ABC佫:+-#😂l😂M😂#-+:𝒳xyz',
      toLowerResult: 'abc佫:+-#😂l😂m😂#-+:𝒳xyz',
      toUpperResult: 'ABC佫:+-#😂L😂M😂#-+:𝒳XYZ'
    }
  ]

  function testStringCaseResult (stringCaseFn, taintedStringCaseFn) {
    let testString = 'abcDEF'

    testString = TaintedUtils.newTaintedString(id, testString, 'PARAM_NAME', 'PARAM_TYPE')
    assert.strictEqual(testString, 'abcDEF', 'Unexpected value')
    assert.equal(true, TaintedUtils.isTainted(id, testString), 'Unexpected value')

    const res = stringCaseFn.call(testString)
    const ret = taintedStringCaseFn(id, res, testString)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(true, TaintedUtils.isTainted(id, ret), 'Unexpected value')
  }

  function testStringCaseNoTaintedResult (stringCaseFn, taintedStringCaseFn) {
    const testString = 'abcDEF'
    const res = stringCaseFn.call(testString)
    const ret = taintedStringCaseFn(id, res, testString)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(false, TaintedUtils.isTainted(id, ret), 'Unexpected value')
  }

  function testStringCaseCheckRanges (stringCaseFn, taintedStringCaseFn, formattedTestString, expectedResult) {
    const testString = taintFormattedString(id, formattedTestString)
    const res = stringCaseFn.call(testString)
    assert.equal(TaintedUtils.isTainted(id, testString), true, 'Test string not tainted')
    const ret = taintedStringCaseFn(id, res, testString)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(TaintedUtils.isTainted(id, ret), true, 'String case returned value not tainted')

    const formattedResult = formatTaintedValue(id, ret)
    assert.equal(formattedResult, expectedResult, 'Unexpected ranges')
  }

  function testStringCaseOneCharResultIsDifferentInstance (stringCaseFn, taintedStringCaseFn, testString) {
    const paramValue = TaintedUtils.newTaintedString(id, testString, 'PARAM_NAME', 'PARAM_TYPE')
    assert.equal(true, TaintedUtils.isTainted(id, paramValue))
    assert.equal(false, TaintedUtils.isTainted(id, testString))
    const originalStringCase = stringCaseFn.call(testString)
    let res = stringCaseFn.call(paramValue)
    res = taintedStringCaseFn(id, res, paramValue)
    assert.equal(true, TaintedUtils.isTainted(id, res))
    assert.equal(false, TaintedUtils.isTainted(id, originalStringCase))
  }

  afterEach(function () {
    TaintedUtils.removeTransaction(id)
  })

  describe('stingCase', function () {
    it('Wrong arguments stringCase', function () {
      assert.throws(function () {
        TaintedUtils.stringCase(id)
      }, Error)
    })
  })

  describe('toLowerCase', function () {
    it('Check result', function () {
      testStringCaseResult(String.prototype.toLowerCase, TaintedUtils.stringCase)
    })

    it('One char length results, different instance', function () {
      testStringCaseOneCharResultIsDifferentInstance(String.prototype.toLowerCase, TaintedUtils.stringCase, 'A')
    })

    it('Check result from not tainted value', function () {
      testStringCaseNoTaintedResult(String.prototype.toLowerCase, TaintedUtils.stringCase)
    })

    it('Secure marks are inherited', () => {
      let op1 = 'HELLO WORLD'
      op1 = TaintedUtils.newTaintedString(id, op1, 'param1', 'REQUEST')
      op1 = TaintedUtils.addSecureMarksToTaintedString(id, op1, 0b0110)

      let result = op1.toLowerCase()
      result = TaintedUtils.stringCase(id, result, op1)

      const ranges = TaintedUtils.getRanges(id, result)
      assert.equal(ranges.length, 1)
      assert.equal(ranges[0].secureMarks, 0b0110)
    })

    describe('Check ranges', function () {
      rangesTestCases.forEach(({ testString, toLowerResult }) => {
        it(`Test ${testString}`, () => {
          testStringCaseCheckRanges(String.prototype.toLowerCase, TaintedUtils.stringCase, testString, toLowerResult)
        })
      })
    })
  })

  describe('toUpperCase', function () {
    it('Check result', function () {
      testStringCaseResult(String.prototype.toUpperCase, TaintedUtils.stringCase)
    })

    it('One char length results, different instance', function () {
      testStringCaseOneCharResultIsDifferentInstance(String.prototype.toUpperCase, TaintedUtils.stringCase, 'A')
    })

    it('Check result from not tainted value', function () {
      testStringCaseNoTaintedResult(String.prototype.toUpperCase, TaintedUtils.stringCase)
    })

    it('Secure marks are inherited', () => {
      let op1 = 'HELLO WORLD'
      op1 = TaintedUtils.newTaintedString(id, op1, 'param1', 'REQUEST')
      op1 = TaintedUtils.addSecureMarksToTaintedString(id, op1, 0b0110)

      let result = op1.toUpperCase()
      result = TaintedUtils.stringCase(id, result, op1)

      const ranges = TaintedUtils.getRanges(id, result)
      assert.equal(ranges.length, 1)
      assert.equal(ranges[0].secureMarks, 0b0110)
    })

    describe('Check ranges', function () {
      rangesTestCases.forEach(({ testString, toUpperResult }) => {
        it(`Test ${testString}`, () => {
          testStringCaseCheckRanges(String.prototype.toUpperCase, TaintedUtils.stringCase, testString, toUpperResult)
        })
      })
    })
  })
})
