/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/

const { TaintedUtils, taintFormattedString, formatTaintedValue } = require('./util')
const assert = require('assert')

const rangesTestCases = [
  {
    source: ':+-foobarbaz-+:',
    result: ':+-foo-+:',
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
    result: ':+-foo-+:',
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
    result: ':+-bar-+:',
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
    result: 'oo:+-ba-+:',
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
    result: 'b:+-a-+:z',
    start: 6,
    end: 9
  },
  {
    source: 'foo:+-bar-+:b:+-a-+:z:+-z-+::+-z-+:',
    result: 'b:+-a-+:z:+-z-+:',
    start: 6,
    end: 10
  },
  {
    source: 'foo:+-bar-+:b:+-a-+:z:+-z-+::+-z-+:',
    result: 'b:+-a-+:z:+-z-+::+-z-+:',
    start: 6,
    end: 20
  }
]

describe('Substring method', function () {
  const id = TaintedUtils.createTransaction('1')

  afterEach(function () {
    TaintedUtils.removeTransaction(id)
  })

  it('Wrong arguments', function () {
    const str = 'hello'
    assert.throws(function () { TaintedUtils.substring(id) }, Error)
    assert.throws(function () { TaintedUtils.substring(id, str.substring()) }, Error)
    assert.doesNotThrow(function () { TaintedUtils.substring(id, str.substring(), 0) }, Error)
  })

  it('String not tainted', function () {
    const op1 = 'hello'
    const start = 1
    const end = op1.length

    const res = op1.substring(start, end)
    const ret = TaintedUtils.substring(id, res, op1, start, end)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(false, TaintedUtils.isTainted(id, ret), 'Unexpected value')
  })

  it('Range not tainted', function () {
    let op1 = 'hello'
    const op2 = ' world'
    const start = op1.length

    op1 = TaintedUtils.newTaintedString(id, op1, 'param', 'REQUEST')

    const res = TaintedUtils.concat(id, op1 + op2, op1, op2)

    let ret = res.substring(start, start)

    ret = TaintedUtils.substring(id, ret, op1, start, res.length)
    assert.equal(false, TaintedUtils.isTainted(id, ret), 'Unexpected value')
  })

  it('Check result', function () {
    let op1 = 'hello'
    const start = 1
    const end = op1.length

    op1 = TaintedUtils.newTaintedString(id, op1, 'param', 'REQUEST')
    assert.strictEqual(op1, 'hello', 'Unexpected value')
    assert.equal(true, TaintedUtils.isTainted(id, op1), 'Unexpected value')

    const res = op1.substring(start, end)
    const ret = TaintedUtils.substring(id, res, op1, start, end)
    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(true, TaintedUtils.isTainted(id, ret), 'Unexpected value')
  })

  it('Check ranges', function () {
    let op1 = 'hello'
    const start = 1
    const end = op1.length

    const res = op1.substring(1, end)

    const expected = [
      {
        start: 0,
        end: res.length,
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

    const ret = TaintedUtils.substring(id, res, op1, start, end)

    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(true, TaintedUtils.isTainted(id, ret), 'Unexpected value')
    assert.deepEqual(expected, TaintedUtils.getRanges(id, ret), 'Unexpected ranges')
  })

  it('Check ranges zero length substring', function () {
    let op1 = 'hello'
    const start = op1.length
    const end = op1.length

    const res = op1.substring(start, end)

    op1 = TaintedUtils.newTaintedString(id, op1, 'param', 'REQUEST')
    assert.strictEqual(op1, 'hello', 'Unexpected value')
    assert.equal(true, TaintedUtils.isTainted(id, op1), 'Unexpected value')

    const ret = TaintedUtils.substring(id, res, op1, start, end)

    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(false, TaintedUtils.isTainted(id, ret), 'Unexpected value')
  })
})

describe('Check Ranges format', function () {
  const id = TaintedUtils.createTransaction('1')

  afterEach(function () {
    TaintedUtils.removeTransaction(id)
  })

  rangesTestCases.forEach(({ source, start, end, result }) => {
    it(`Test ${source}`, function () {
      const inputString = taintFormattedString(id, source)
      assert.equal(TaintedUtils.isTainted(id, inputString), true, 'Not tainted')
      const res = inputString.substring(start, end)

      const ret = TaintedUtils.substring(id, res, inputString, start, end)
      assert.equal(res, ret, 'Unexpected value')

      const formattedResult = formatTaintedValue(id, ret)
      assert.equal(formattedResult, result, 'Unexpected ranges')
    })
  })
})
