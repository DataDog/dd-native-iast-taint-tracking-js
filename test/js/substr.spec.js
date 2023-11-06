/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/

const { TaintedUtils, taintFormattedString, formatTaintedValue } = require('./util')
const assert = require('assert')

const rangesTestCases = [
  {
    source: ':+-foobarbaz-+:',
    result: '',
    start: 3,
    end: 0
  },
  {
    source: ':+-foobarbaz-+:',
    result: '',
    start: -6,
    end: -3
  },
  {
    source: ':+-foobarbaz-+:',
    result: ':+-foo-+:',
    start: 0,
    end: 3
  },
  {
    source: ':+-foobarbaz-+:',
    result: ':+-barbaz-+:',
    start: 3,
    end: undefined
  },
  {
    source: ':+-foobarbaz-+:',
    result: ':+-barbaz-+:',
    start: 3
  },
  {
    source: ':+-foobarbaz-+:',
    result: ':+-bar-+:',
    start: -6,
    end: 3
  },
  {
    source: ':+-foobarbaz-+:',
    result: ':+-foo-+:',
    start: -9,
    end: 3
  },
  {
    source: ':+-foobarbaz-+:',
    result: ':+-barbaz-+:',
    start: 3,
    end: 6
  },
  {
    source: ':+-foobarbaz-+:',
    result: ':+-baz-+:',
    start: 6,
    end: 9
  },
  {
    source: ':+-foobarbaz-+:',
    result: ':+-baz-+:',
    start: 6
  },
  {
    source: 'foo:+-bar-+:baz',
    result: 'foo',
    start: 0,
    end: 3
  },
  {
    source: 'foo:+-bar-+:baz',
    result: 'oo:+-bar-+:',
    start: 1,
    end: 5
  },
  {
    source: 'foo:+-bar-+:baz',
    result: ':+-r-+:baz',
    start: 5,
    end: 9
  },
  {
    source: 'foo:+-bar-+:baz',
    result: 'baz',
    start: 6,
    end: 9
  },
  {
    source: 'foo:+-bar-+:b:+-az-+:',
    result: 'b:+-az-+:',
    start: 6,
    end: 9
  },
  {
    source: 'foo:+-bar-+:b:+-a-+:z',
    result: 'b:+-a-+:z',
    start: 6,
    end: 9
  },
  {
    source: 'foo:+-bar-+:b:+-a-+:z',
    result: 'b:+-a-+:z',
    start: 6,
    end: 15
  },
  {
    source: 'foo:+-bar-+:b:+-a-+:z:+-z-+::+-z-+:',
    result: 'b:+-a-+:z:+-z-+::+-z-+:',
    start: 6,
    end: 9
  },
  {
    source: 'foo:+-bar-+:b:+-a-+:z:+-z-+::+-z-+:',
    result: 'b:+-a-+:z:+-z-+::+-z-+:',
    start: 6,
    end: 10
  },
  {
    source: 'foo:+-bar-+:b:+-a-+:z:+-z-+::+-z-+:',
    result: 'b:+-a-+:z:+-z-+::+-z-+:',
    start: 6,
    end: 20
  },
  {
    source: 'foo:+-bar-+:b:+-a-+:z:+-z-+::+-z-+:',
    result: 'b:+-a-+:z:+-z-+::+-z-+:',
    start: 6,
    end: undefined
  },
  {
    source: 'foo:+-bar-+:b:+-a-+:z:+-z-+::+-z-+:',
    result: 'b:+-a-+:z:+-z-+::+-z-+:',
    start: 6
  },
  {
    source: ':+-bar-+:foo:+-baz-+:',
    result: '',
    start: 10,
    end: 7
  },
  {
    source: ':+-bar-+:foo:+-baz-+:',
    result: ':+-bar-+:foo:+-b-+:',
    start: -200,
    end: 7
  },
  {
    source: ':+-bar-+:foo:+-baz-+:',
    result: '',
    start: 5,
    end: -30
  },
  {
    source: ':+-bar-+:foo:+-baz-+:',
    result: '',
    start: 5,
    end: -3
  },
  {
    source: ':+-bar-+:foo:+-baz-+:',
    result: 'o:+-baz-+:',
    start: 5,
    end: 200
  }
]

describe('Substr method', function () {
  const id = TaintedUtils.createTransaction('1')

  afterEach(function () {
    TaintedUtils.removeTransaction(id)
  })

  it('Wrong arguments', function () {
    const str = 'hello'
    assert.throws(function () { TaintedUtils.substr(id) }, Error)
    assert.throws(function () { TaintedUtils.substr(id, str.substr()) }, Error)
    assert.doesNotThrow(function () { TaintedUtils.substr(id, str.substr(), 0) }, Error)
  })

  it('String not tainted', function () {
    const op1 = 'hello'
    const start = 1
    const end = op1.length

    const res = op1.substr(start, end)
    const ret = TaintedUtils.substr(id, res, op1, start, end)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(false, TaintedUtils.isTainted(id, ret), 'Unexpected value')
  })

  it('Range not tainted', function () {
    let op1 = 'hello'
    const op2 = ' world'
    const start = op1.length

    op1 = TaintedUtils.newTaintedString(id, op1, 'param', 'REQUEST')

    const res = TaintedUtils.concat(id, op1 + op2, op1, op2)

    let ret = res.substr(start, start)

    ret = TaintedUtils.substr(id, ret, op1, start, res.length)
    assert.equal(false, TaintedUtils.isTainted(id, ret), 'Unexpected value')
  })

  it('Check result', function () {
    let op1 = 'hello'
    const start = 1
    const end = op1.length

    op1 = TaintedUtils.newTaintedString(id, op1, 'param', 'REQUEST')
    assert.strictEqual(op1, 'hello', 'Unexpected value')
    assert.equal(true, TaintedUtils.isTainted(id, op1), 'Unexpected value')

    const res = op1.substr(start, end)
    const ret = TaintedUtils.substr(id, res, op1, start, end)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(true, TaintedUtils.isTainted(id, ret), 'Unexpected value')
  })

  it('Check ranges', function () {
    let op1 = 'hello'
    const start = 1
    const end = op1.length

    const res = op1.substr(1, end)

    const expected = [
      {
        start: 0,
        end: res.length,
        iinfo:
                {
                  parameterName: 'param',
                  parameterValue: 'hello',
                  type: 'REQUEST'
                },
        secureMarks: 0
      }
    ]

    op1 = TaintedUtils.newTaintedString(id, op1, 'param', 'REQUEST')
    assert.strictEqual(op1, 'hello', 'Unexpected value')
    assert.equal(true, TaintedUtils.isTainted(id, op1), 'Unexpected value')

    const ret = TaintedUtils.substr(id, res, op1, start, end)

    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(true, TaintedUtils.isTainted(id, ret), 'Unexpected value')
    assert.deepEqual(expected, TaintedUtils.getRanges(id, ret), 'Unexpected ranges')
  })

  it('Check ranges zero length substring', function () {
    let op1 = 'hello'
    const start = op1.length
    const end = op1.length

    const res = op1.substr(start, end)

    op1 = TaintedUtils.newTaintedString(id, op1, 'param', 'REQUEST')
    assert.strictEqual(op1, 'hello', 'Unexpected value')
    assert.equal(true, TaintedUtils.isTainted(id, op1), 'Unexpected value')

    const ret = TaintedUtils.substr(id, res, op1, start, end)

    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(false, TaintedUtils.isTainted(id, ret), 'Unexpected value')
  })

  it('Secure marks are inherited', () => {
    let op1 = 'hello world'
    op1 = TaintedUtils.newTaintedString(id, op1, 'param1', 'REQUEST')
    op1 = TaintedUtils.addSecureMarksToTaintedString(id, op1, 0b0110)

    let result = op1.substr(6)
    result = TaintedUtils.substr(id, result, op1, 6)

    const ranges = TaintedUtils.getRanges(id, result)
    assert.equal(ranges.length, 1)
    assert.equal(ranges[0].secureMarks, 0b0110)
  })
})

describe('Check Ranges format', function () {
  const id = TaintedUtils.createTransaction('1')

  afterEach(function () {
    TaintedUtils.removeTransaction(id)
  })

  rangesTestCases.forEach((testData) => {
    const { source, start, end, result } = testData

    it(`Test ${source}`, function () {
      const inputString = taintFormattedString(id, source)
      assert.equal(TaintedUtils.isTainted(id, inputString), true, 'Not tainted')
      const res = inputString.substr(start, end)

      const ret = testData.hasOwnProperty('end')
        ? TaintedUtils.substr(id, res, inputString, start, end)
        : TaintedUtils.substr(id, res, inputString, start)
      assert.equal(res, ret, 'Unexpected value')

      const formattedResult = formatTaintedValue(id, ret)
      assert.equal(formattedResult, result, 'Unexpected ranges')
    })
  })
})
