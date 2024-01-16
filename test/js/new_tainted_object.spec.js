/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
const { TaintedUtils } = require('./util')
const assert = require('assert')

describe('Taint objects', function () {
  const value = Buffer.from('test')
  const id = TaintedUtils.createTransaction('1')

  before(() => {
    TaintedUtils.setMaxTransactions(1)
  })

  afterEach(function () {
    TaintedUtils.removeTransaction(id)
  })

  after(() => {
    TaintedUtils.setMaxTransactions(2)
  })

  it('Taint buffer with undefined transaction', function () {
    const ret = TaintedUtils.newTaintedObject(undefined, value, 'param', 'REQUEST')
    assert.strictEqual(ret, value, 'Unexpected value')
  })

  it('Taint buffer with empty transaction', function () {
    const ret = TaintedUtils.newTaintedObject('', value, 'param', 'REQUEST')
    assert.strictEqual(ret, value, 'Unexpected value')
  })

  it('Taint buffer with valid transaction', function () {
    const ret = TaintedUtils.newTaintedObject(id, value, 'param', 'REQUEST')
    assert.strictEqual(ret, value, 'Unexpected value')
  })

  it('Taint string through newTaintedObject method', function () {
    const value = 'test'
    const ret = TaintedUtils.newTaintedObject(id, value, 'param', 'REQUEST')
    assert.strictEqual(ret, value, 'Unexpected value')

    // a new string is created when value length < 10
    assert.strictEqual(true, TaintedUtils.isTainted(id, ret), 'value should be tainted')

    const ranges = TaintedUtils.getRanges(id, ret)

    assert.strictEqual(ranges.length, 1, 'Unexpected ranges length')

    assert.deepEqual(ranges[0], {
      start: 0,
      end: ret.length,
      iinfo: {
        parameterName: 'param',
        parameterValue: ret,
        type: 'REQUEST'
      },
      secureMarks: 0
    })
  })

  it('Check tainted object', function () {
    const taintedValue = TaintedUtils.newTaintedObject(id, value, 'param', 'REQUEST')
    const ret = TaintedUtils.isTainted(id, taintedValue)
    assert.strictEqual(ret, true, 'Unexpected value')
  })

  it('Check tainted String object', function () {
    // eslint-disable-next-line no-new-wrappers
    const taintedValue = TaintedUtils.newTaintedObject(id, new String('test'), 'param', 'REQUEST')
    const ret = TaintedUtils.isTainted(id, taintedValue)
    assert.strictEqual(ret, true, 'Unexpected value')
  })

  it('Check tainted object ranges', function () {
    const taintedValue = TaintedUtils.newTaintedObject(id, value, 'param', 'REQUEST')
    const ret = TaintedUtils.isTainted(id, taintedValue)
    assert.strictEqual(ret, true, 'Unexpected value')

    const ranges = TaintedUtils.getRanges(id, value)

    assert.strictEqual(ranges.length, 1, 'Unexpected value')

    assert.deepEqual(ranges[0], {
      start: 0,
      end: 4,
      iinfo: {
        parameterName: 'param',
        parameterValue: value,
        type: 'REQUEST'
      },
      secureMarks: 0
    })
  })
})
