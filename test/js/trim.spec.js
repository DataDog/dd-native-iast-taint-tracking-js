/**
 * Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
 * This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
 **/
const { TaintedUtils, taintFormattedString, formatTaintedValue } = require('./util')
const assert = require('assert')

describe('Trim operator', function () {
  const id = TaintedUtils.createTransaction('1')

  const noTaintTestCases = {
    trim: [
      ':+-    -+:',
      ':+- \n\r\t -+:',
      ':+-    -+:ABC:+-    -+:',
      ' :+-    -+:ABC:+-    -+: ',
      ' :+-    -+: ABC :+-    -+: '
    ],
    trimStart: [
      ':+-    -+:',
      ':+- \n\r\t -+:',
      ':+-    -+:ABC',
      ' :+-    -+: ABC ',
      ' :+-    -+: :+-    -+: ABC'
    ],
    trimEnd: [
      ':+-    -+:',
      ':+- \n\r\t -+:',
      'ABC:+-    -+:',
      'ABC :+-    -+: ',
      'ABC :+-    -+: :+-    -+: '
    ]
  }

  const rangesTestCases = [
    {
      testString: ':+-ABC-+:',
      trimResult: ':+-ABC-+:',
      trimStartResult: ':+-ABC-+:',
      trimEndResult: ':+-ABC-+:'
    },
    {
      testString: ':+-ABC-+:   ',
      trimResult: ':+-ABC-+:',
      trimStartResult: ':+-ABC-+:   ',
      trimEndResult: ':+-ABC-+:'
    },
    {
      testString: '   :+-ABC-+:',
      trimResult: ':+-ABC-+:',
      trimStartResult: ':+-ABC-+:',
      trimEndResult: '   :+-ABC-+:'
    },
    {
      testString: '   :+-ABC-+:   ',
      trimResult: ':+-ABC-+:',
      trimStartResult: ':+-ABC-+:   ',
      trimEndResult: '   :+-ABC-+:'
    },
    {
      testString: '   \n   :+-ABC-+:   \n   ',
      trimResult: ':+-ABC-+:',
      trimStartResult: ':+-ABC-+:   \n   ',
      trimEndResult: '   \n   :+-ABC-+:'
    },
    {
      testString: '   \t   :+-ABC-+:   \t   ',
      trimResult: ':+-ABC-+:',
      trimStartResult: ':+-ABC-+:   \t   ',
      trimEndResult: '   \t   :+-ABC-+:'
    },
    {
      testString: '   \r   :+-ABC-+:   \r   ',
      trimResult: ':+-ABC-+:',
      trimStartResult: ':+-ABC-+:   \r   ',
      trimEndResult: '   \r   :+-ABC-+:'
    },
    {
      testString: ':+-   -+::+-ABC-+:',
      trimResult: ':+-ABC-+:',
      trimStartResult: ':+-ABC-+:',
      trimEndResult: ':+-   -+::+-ABC-+:'
    },
    {
      testString: ':+-  12-+::+-ABC-+:',
      trimResult: ':+-12-+::+-ABC-+:',
      trimStartResult: ':+-12-+::+-ABC-+:',
      trimEndResult: ':+-  12-+::+-ABC-+:'
    },
    {
      testString: ' :+-  12-+::+-ABC-+:',
      trimResult: ':+-12-+::+-ABC-+:',
      trimStartResult: ':+-12-+::+-ABC-+:',
      trimEndResult: ' :+-  12-+::+-ABC-+:'
    },
    {
      testString: ':+-   -+: :+-ABC-+:',
      trimResult: ':+-ABC-+:',
      trimStartResult: ':+-ABC-+:',
      trimEndResult: ':+-   -+: :+-ABC-+:'
    },
    {
      testString: ':+-   -+: :+-ABC-+: ',
      trimResult: ':+-ABC-+:',
      trimStartResult: ':+-ABC-+: ',
      trimEndResult: ':+-   -+: :+-ABC-+:'
    },
    {
      testString: ':+-ABC-+::+-   -+:',
      trimResult: ':+-ABC-+:',
      trimStartResult: ':+-ABC-+::+-   -+:',
      trimEndResult: ':+-ABC-+:'
    },
    {
      testString: ':+-ABC-+::+-12  -+:',
      trimResult: ':+-ABC-+::+-12-+:',
      trimStartResult: ':+-ABC-+::+-12  -+:',
      trimEndResult: ':+-ABC-+::+-12-+:'
    },
    {
      testString: ':+-ABC-+::+-12  -+: ',
      trimResult: ':+-ABC-+::+-12-+:',
      trimStartResult: ':+-ABC-+::+-12  -+: ',
      trimEndResult: ':+-ABC-+::+-12-+:'
    },
    {
      testString: ':+-ABC-+: :+-   -+:',
      trimResult: ':+-ABC-+:',
      trimStartResult: ':+-ABC-+: :+-   -+:',
      trimEndResult: ':+-ABC-+:'
    },
    {
      testString: ' :+-ABC-+: :+-   -+:',
      trimResult: ':+-ABC-+:',
      trimStartResult: ':+-ABC-+: :+-   -+:',
      trimEndResult: ' :+-ABC-+:'
    },
    {
      testString: ':+-ABC-+:12345:+-   -+:',
      trimResult: ':+-ABC-+:12345',
      trimStartResult: ':+-ABC-+:12345:+-   -+:',
      trimEndResult: ':+-ABC-+:12345'
    },
    {
      testString: ':+-ABC-+:12345   :+-   -+: ',
      trimResult: ':+-ABC-+:12345',
      trimStartResult: ':+-ABC-+:12345   :+-   -+: ',
      trimEndResult: ':+-ABC-+:12345'
    },
    {
      testString: ':+-ABC-+::+-   -+:12345    ',
      trimResult: ':+-ABC-+::+-   -+:12345',
      trimStartResult: ':+-ABC-+::+-   -+:12345    ',
      trimEndResult: ':+-ABC-+::+-   -+:12345'
    },
    {
      testString: ':+-   -+:12345:+-ABC-+:',
      trimResult: '12345:+-ABC-+:',
      trimStartResult: '12345:+-ABC-+:',
      trimEndResult: ':+-   -+:12345:+-ABC-+:'
    },
    {
      testString: ' :+-   -+:   12345:+-ABC-+:',
      trimResult: '12345:+-ABC-+:',
      trimStartResult: '12345:+-ABC-+:',
      trimEndResult: ' :+-   -+:   12345:+-ABC-+:'
    },
    {
      testString: '   12345:+-   -+::+-ABC-+:',
      trimResult: '12345:+-   -+::+-ABC-+:',
      trimStartResult: '12345:+-   -+::+-ABC-+:',
      trimEndResult: '   12345:+-   -+::+-ABC-+:'
    },
    {
      testString: '   12345:+-   -+::+-ABC-+::+-   -+:67890   ',
      trimResult: '12345:+-   -+::+-ABC-+::+-   -+:67890',
      trimStartResult: '12345:+-   -+::+-ABC-+::+-   -+:67890   ',
      trimEndResult: '   12345:+-   -+::+-ABC-+::+-   -+:67890'
    },
    {
      testString: '   \n   12345:+-   -+::+-ABC-+::+-   -+:67890   \n   ',
      trimResult: '12345:+-   -+::+-ABC-+::+-   -+:67890',
      trimStartResult: '12345:+-   -+::+-ABC-+::+-   -+:67890   \n   ',
      trimEndResult: '   \n   12345:+-   -+::+-ABC-+::+-   -+:67890'
    },
    {
      testString: '   \t   12345:+-   -+::+-ABC-+::+-   -+:67890   \t   ',
      trimResult: '12345:+-   -+::+-ABC-+::+-   -+:67890',
      trimStartResult: '12345:+-   -+::+-ABC-+::+-   -+:67890   \t   ',
      trimEndResult: '   \t   12345:+-   -+::+-ABC-+::+-   -+:67890'
    },
    {
      testString: '   \r   12345:+-   -+::+-ABC-+::+-   -+:67890   \r   ',
      trimResult: '12345:+-   -+::+-ABC-+::+-   -+:67890',
      trimStartResult: '12345:+-   -+::+-ABC-+::+-   -+:67890   \r   ',
      trimEndResult: '   \r   12345:+-   -+::+-ABC-+::+-   -+:67890'
    },
    {
      testString: ':+-12345-+::+-ABC-+::+-67890-+:',
      trimResult: ':+-12345-+::+-ABC-+::+-67890-+:',
      trimStartResult: ':+-12345-+::+-ABC-+::+-67890-+:',
      trimEndResult: ':+-12345-+::+-ABC-+::+-67890-+:'
    },
    {
      testString: ' :+-  12345-+: :+-ABC-+: :+-67890  -+: ',
      trimResult: ':+-12345-+: :+-ABC-+: :+-67890-+:',
      trimStartResult: ':+-12345-+: :+-ABC-+: :+-67890  -+: ',
      trimEndResult: ' :+-  12345-+: :+-ABC-+: :+-67890-+:'
    },
    {
      testString: ':+-   -+: :+-   -+: :+-ABC-+: ',
      trimResult: ':+-ABC-+:',
      trimStartResult: ':+-ABC-+: ',
      trimEndResult: ':+-   -+: :+-   -+: :+-ABC-+:'
    },
    {
      testString: ' :+-   -+: :+-   -+: :+-ABC-+: ',
      trimResult: ':+-ABC-+:',
      trimStartResult: ':+-ABC-+: ',
      trimEndResult: ' :+-   -+: :+-   -+: :+-ABC-+:'
    },
    {
      testString: ' :+-   -+: :+-   -+: :+-ABC-+: :+-   -+: ',
      trimResult: ':+-ABC-+:',
      trimStartResult: ':+-ABC-+: :+-   -+: ',
      trimEndResult: ' :+-   -+: :+-   -+: :+-ABC-+:'
    },
    {
      testString: ' :+-   -+: :+-   -+: :+-ABC-+: :+-   -+: :+-   -+: ',
      trimResult: ':+-ABC-+:',
      trimStartResult: ':+-ABC-+: :+-   -+: :+-   -+: ',
      trimEndResult: ' :+-   -+: :+-   -+: :+-ABC-+:'
    },
    {
      testString: ' :+-   -+: :+-   -+: :+-  ABC-+: :+-1234567890  -+: :+-   -+: :+-   -+: ',
      trimResult: ':+-ABC-+: :+-1234567890-+:',
      trimStartResult: ':+-ABC-+: :+-1234567890  -+: :+-   -+: :+-   -+: ',
      trimEndResult: ' :+-   -+: :+-   -+: :+-  ABC-+: :+-1234567890-+:'
    },
    {
      testString: '   佫:+-   -+::+-😂😂😂-+::+-   -+:𝒳   ',
      trimResult: '佫:+-   -+::+-😂😂😂-+::+-   -+:𝒳',
      trimStartResult: '佫:+-   -+::+-😂😂😂-+::+-   -+:𝒳   ',
      trimEndResult: '   佫:+-   -+::+-😂😂😂-+::+-   -+:𝒳'
    },
    {
      testString: ' :+-  佫-+: :+-😂😂😂-+: :+-𝒳  -+: ',
      trimResult: ':+-佫-+: :+-😂😂😂-+: :+-𝒳-+:',
      trimStartResult: ':+-佫-+: :+-😂😂😂-+: :+-𝒳  -+: ',
      trimEndResult: ' :+-  佫-+: :+-😂😂😂-+: :+-𝒳-+:'
    }
  ]

  function testTrimResult (trimFn, taintedTrimFn) {
    let testString = '   ABC   '

    testString = TaintedUtils.newTaintedString(id, testString, 'PARAM_NAME', 'PARAM_TYPE')
    assert.strictEqual(testString, '   ABC   ', 'Unexpected value')
    assert.equal(true, TaintedUtils.isTainted(id, testString), 'Unexpected value')

    const res = trimFn.call(testString)
    const ret = taintedTrimFn(id, res, testString)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(true, TaintedUtils.isTainted(id, ret), 'Unexpected value')
  }

  function testTrimNoTaintedResult (trimFn, taintedTrimFn) {
    const testString = '   ABC   '
    const res = trimFn.call(testString)
    const ret = taintedTrimFn(id, res, testString)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(false, TaintedUtils.isTainted(id, ret), 'Unexpected value')
  }

  function testTrimCheckRanges (trimFn, taintedTrimFn, formattedTestString, expectedResult) {
    const testString = taintFormattedString(id, formattedTestString)
    const res = trimFn.call(testString)
    assert.equal(TaintedUtils.isTainted(id, testString), true, 'Test string not tainted')
    const ret = taintedTrimFn(id, res, testString)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(TaintedUtils.isTainted(id, ret), true, 'Trim returned value not tainted')

    const formattedResult = formatTaintedValue(id, ret)
    assert.equal(formattedResult, expectedResult, 'Unexpected ranges')
  }

  function testTrimNoTaintedWhenAllRangesTrimmed (trimFn, taintedTrimFn, formattedTestString) {
    const testString = taintFormattedString(id, formattedTestString)
    const res = trimFn.call(testString)
    assert.equal(TaintedUtils.isTainted(id, testString), true, 'Test string not tainted')
    const ret = taintedTrimFn(id, res, testString)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(TaintedUtils.isTainted(id, ret), false, 'Trim returned value is tainted')
  }

  function testTrimOneCharResultIsDifferentInstance (trimFn, taintedTrimFn, testString) {
    const paramValue = TaintedUtils.newTaintedString(id, testString, 'PARAM_NAME', 'PARAM_TYPE')
    assert.equal(true, TaintedUtils.isTainted(id, paramValue))
    assert.equal(false, TaintedUtils.isTainted(id, testString))
    const originalTrim = trimFn.call(testString)
    let res = trimFn.call(paramValue)
    res = taintedTrimFn(id, res, paramValue)
    assert.equal(true, TaintedUtils.isTainted(id, res))
    assert.equal(false, TaintedUtils.isTainted(id, originalTrim))
  }

  afterEach(function () {
    TaintedUtils.removeTransaction(id)
  })

  describe('trim', function () {
    it('Wrong arguments trim', function () {
      assert.throws(function () {
        TaintedUtils.trim(id)
      }, Error)
    })

    it('Check result', function () {
      testTrimResult(String.prototype.trim, TaintedUtils.trim)
    })

    it('One char length results, different instance', function () {
      testTrimOneCharResultIsDifferentInstance(String.prototype.trim, TaintedUtils.trim, ' a ')
    })

    it('Check result from not tainted value', function () {
      testTrimNoTaintedResult(String.prototype.trim, TaintedUtils.trim)
    })

    it('Secure marks are inherited', () => {
      let op1 = '   hello world   '
      op1 = TaintedUtils.newTaintedString(id, op1, 'param1', 'REQUEST')
      op1 = TaintedUtils.addSecureMarksToTaintedString(id, op1, 0b0110)

      let result = op1.trim()
      result = TaintedUtils.trim(id, result, op1)

      const ranges = TaintedUtils.getRanges(id, result)
      assert.equal(ranges.length, 1)
      assert.equal(ranges[0].secureMarks, 0b0110)
    })

    describe('Check result not tainted when no ranges left', function () {
      noTaintTestCases.trim.forEach((testString) => {
        it(`Test ${testString}`, () => {
          testTrimNoTaintedWhenAllRangesTrimmed(String.prototype.trim, TaintedUtils.trim, testString)
        })
      })
    })

    describe('Check ranges', function () {
      rangesTestCases.forEach(({ testString, trimResult }) => {
        it(`Test ${testString}`, () => {
          testTrimCheckRanges(String.prototype.trim, TaintedUtils.trim, testString, trimResult)
        })
      })
    })
  })

  describe('trimStart', function () {
    it('Check result', function () {
      testTrimResult(String.prototype.trimStart, TaintedUtils.trim)
    })

    it('Check result from not tainted value', function () {
      testTrimNoTaintedResult(String.prototype.trimStart, TaintedUtils.trim)
    })

    it('One char length results, different instance', function () {
      testTrimOneCharResultIsDifferentInstance(String.prototype.trimStart, TaintedUtils.trim, ' a')
    })

    describe('Check result not tainted when no ranges left', function () {
      noTaintTestCases.trimStart.forEach((testString) => {
        it(`Test ${testString}`, () => {
          testTrimNoTaintedWhenAllRangesTrimmed(String.prototype.trimStart, TaintedUtils.trim, testString)
        })
      })
    })

    describe('Check ranges', function () {
      rangesTestCases.forEach(({ testString, trimStartResult }) => {
        it(`Test ${testString}`, () => {
          testTrimCheckRanges(String.prototype.trimStart, TaintedUtils.trim, testString, trimStartResult)
        })
      })
    })
  })

  describe('trimEnd', function () {
    it('Wrong arguments trimEnd', function () {
      assert.throws(function () {
        TaintedUtils.trimEnd(id)
      }, Error)
    })

    it('Check result', function () {
      testTrimResult(String.prototype.trimEnd, TaintedUtils.trimEnd)
    })

    it('Check result from not tainted value', function () {
      testTrimNoTaintedResult(String.prototype.trimEnd, TaintedUtils.trimEnd)
    })

    it('One char length results, different instance', function () {
      testTrimOneCharResultIsDifferentInstance(String.prototype.trimEnd, TaintedUtils.trimEnd, 'a  ')
    })

    describe('Check result not tainted when no ranges left', function () {
      noTaintTestCases.trimEnd.forEach((testString) => {
        it(`Test ${testString}`, () => {
          testTrimNoTaintedWhenAllRangesTrimmed(String.prototype.trimEnd, TaintedUtils.trimEnd, testString)
        })
      })
    })

    describe('Check ranges', function () {
      rangesTestCases.forEach(({ testString, trimEndResult }) => {
        it(`Test ${testString}`, () => {
          testTrimCheckRanges(String.prototype.trimEnd, TaintedUtils.trimEnd, testString, trimEndResult)
        })
      })
    })
  })
})
