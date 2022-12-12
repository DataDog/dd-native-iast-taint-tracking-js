/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/

const { TaintedUtils, taintFormattedString, formatTaintedValue } = require('./util')
const assert = require('assert')

const rangesTestCases = [
  {
    string: ':+-foobar-+:',
    start: 0,
    end: 3,
    result: ':+-foo-+:'
  },
  {
    string: ':+-foo-+:bar',
    start: 3,
    end: 6,
    result: 'bar'
  },
  {
    string: ':+-foo-+:bar',
    start: 0,
    end: 2,
    result: ':+-fo-+:'
  },
  {
    string: ':+-foo-+:bar',
    start: 1,
    end: 4,
    result: ':+-oo-+:b'
  },
  {
    string: ':+-foo-+:bar:+-baz-+:',
    start: 3,
    end: 8,
    result: 'bar:+-ba-+:'
  }
]

function substringCheckRanges (id, testString, start, end, expected) {
  const inputString = taintFormattedString(id, testString)
  assert.equal(TaintedUtils.isTainted(id, inputString), true, 'Not tainted')
  const res = inputString.substring(start, end)

  assert.equal(TaintedUtils.isTainted(id, inputString), true, 'Not tainted')
  const ret = TaintedUtils.substring(id, res, inputString, start, end)
  assert.equal(res, ret, 'Unexpected value')

  const formattedResult = formatTaintedValue(id, ret)
  assert.equal(formattedResult, expected, 'Unexpected ranges')
}

describe('Substring method', function () {
  const id = '1'

  afterEach(function () {
    TaintedUtils.removeTransaction(id)
  })

  it('Wrong arguments', function () {
    assert.throws(function () { TaintedUtils.substring(id) }, Error)
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

    const res = op1.substr(start, end)

    op1 = TaintedUtils.newTaintedString(id, op1, 'param', 'REQUEST')
    assert.strictEqual(op1, 'hello', 'Unexpected value')
    assert.equal(true, TaintedUtils.isTainted(id, op1), 'Unexpected value')

    const ret = TaintedUtils.substring(id, res, op1, start, end)

    assert.equal(res, ret, 'Unexpected vale')
    assert.equal(false, TaintedUtils.isTainted(id, ret), 'Unexpected value')
  })
})

describe('Check Ranges format', function () {
  const id = '1'
  rangesTestCases.forEach(({ string, start, end, result }) => {
    it(`Test ${string}`, function () {
      substringCheckRanges(id, string, start, end, result)
    })
  })
})
