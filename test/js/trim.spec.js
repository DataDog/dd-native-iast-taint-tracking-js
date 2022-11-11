/**
 * Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
 * This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
 **/
const { TaintedUtils, taintFormattedString, formatTaintedValue } = require('./util')
const assert = require('assert')

describe('Trim operator', function () {
  const id = '1'

  const testCases = [
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

  afterEach(function () {
    TaintedUtils.removeTransaction(id)
  })

  it('Wrong arguments', function () {
    assert.throws(function () {
      TaintedUtils.concat(id)
    }, Error)
  })

  describe('trim', function () {
    it('Check result', function () {
      testTrimResult(String.prototype.trim, TaintedUtils.trim)
    })

    describe('Check ranges', function () {
      testCases.forEach(({ testString, trimResult }) => {
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

    describe('Check ranges', function () {
      testCases.forEach(({ testString, trimStartResult }) => {
        it(`Test ${testString}`, () => {
          testTrimCheckRanges(String.prototype.trimStart, TaintedUtils.trim, testString, trimStartResult)
        })
      })
    })
  })

  describe('trimEnd', function () {
    it('Check result', function () {
      testTrimResult(String.prototype.trimEnd, TaintedUtils.trimEnd)
    })

    describe('Check ranges', function () {
      testCases.forEach(({ testString, trimEndResult }) => {
        it(`Test ${testString}`, () => {
          testTrimCheckRanges(String.prototype.trimEnd, TaintedUtils.trimEnd, testString, trimEndResult)
        })
      })
    })
  })
})
