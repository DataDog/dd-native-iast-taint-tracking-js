/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
const { TaintedUtils } = require('./util')
const assert = require('assert')

const MAX_TAINTED_OBJECTS = 4096

describe('Taint strings', function () {
  const value = 'test'
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

  it('Taint new string with undefined transaction', function () {
    const ret = TaintedUtils.newTaintedString(undefined, value, 'param', 'REQUEST')
    assert.strictEqual(ret, 'test', 'Unexpected value')
  })

  it('Taint new string with empty transaction', function () {
    const ret = TaintedUtils.newTaintedString('', value, 'param', 'REQUEST')
    assert.strictEqual(ret, 'test', 'Unexpected value')
  })

  it('Taint new string', function () {
    const ret = TaintedUtils.newTaintedString(id, value, 'param', 'REQUEST')
    assert.strictEqual(ret, 'test', 'Unexpected value')
  })

  it('Wrong number of args', function () {
    assert.throws(function () { TaintedUtils.newTaintedString(id, value) }, Error)
    assert.throws(function () { TaintedUtils.isTainted(id) }, Error)
  })

  it('Check untainted string', function () {
    const ret = TaintedUtils.isTainted(id, value)
    assert.strictEqual(ret, false, 'Unexpected value')
  })

  it('Check tainted string', function () {
    const taintedValue = TaintedUtils.newTaintedString(id, value, 'param', 'REQUEST')
    const ret = TaintedUtils.isTainted(id, taintedValue)
    assert.strictEqual(ret, true, 'Unexpected value')
  })

  it('Forces new string', function () {
    const taintedValue = TaintedUtils.newTaintedString(id, value, 'param', 'REQUEST')
    assert.strictEqual(TaintedUtils.isTainted(id, taintedValue), true, 'Unexpected value')
    assert.strictEqual(TaintedUtils.isTainted(id, value), false, 'Unexpected value')
  })

  it('Check tainted string with unknown id', function () {
    const wrongId = '2'

    const taintedValue = TaintedUtils.newTaintedString(id, value, 'param', 'REQUEST')
    const ret = TaintedUtils.isTainted(wrongId, taintedValue)
    assert.strictEqual(ret, false, 'Unexpected value')
  })

  it('Taint number', function () {
    let ret
    const value = 123

    ret = TaintedUtils.newTaintedString(id, value, 'param', 'REQUEST')
    assert.strictEqual(ret, 123, 'Unexpected value')
    ret = TaintedUtils.isTainted(id, ret)
    assert.strictEqual(ret, false, 'Unexpected value')
  })

  it('Taint object', function () {
    let ret
    const value = {}

    ret = TaintedUtils.newTaintedString(id, value, 'param', 'REQUEST')
    assert.strictEqual(ret, value, 'Unexpected value')
    ret = TaintedUtils.isTainted(id, ret)
    assert.strictEqual(ret, false, 'Unexpected value')
  })

  it('Taint undefined', function () {
    let ret
    let value

    ret = TaintedUtils.newTaintedString(id, value, 'param', 'REQUEST')
    assert.strictEqual(ret, undefined, 'Unexpected value')
    ret = TaintedUtils.isTainted(id, ret)
    assert.strictEqual(ret, false, 'Unexpected value')
  })

  it('Taint null', function () {
    let ret
    const value = null

    ret = TaintedUtils.newTaintedString(id, value, 'param', 'REQUEST')
    assert.strictEqual(ret, null, 'Unexpected value')
    ret = TaintedUtils.isTainted(id, ret)
    assert.strictEqual(ret, false, 'Unexpected value')
  })

  it('Max values', function () {
    let ret
    const values = new Array(MAX_TAINTED_OBJECTS).fill('value')

    values.forEach((val, index, array) => {
      ret = TaintedUtils.newTaintedString(id, val, 'param', 'REQUEST')
      assert.strictEqual(ret, 'value', 'Unexpected value')
      ret = TaintedUtils.isTainted(id, ret)
      assert.strictEqual(ret, true, 'Unexpected value')
    })
  })

  it('Beyond max concurrent transactions', function () {
    const MAX_TRANSACTION_MAP_SIZE = 2
    const transactions = []
    for (let i = 1; i < MAX_TRANSACTION_MAP_SIZE + 1; i++) {
      transactions[i] = TaintedUtils.createTransaction(`beyondTest${i}`)
      TaintedUtils.newTaintedString(transactions[i], `taintedString${i}`, 'param', 'request')
    }

    transactions.forEach((transactionId) => TaintedUtils.removeTransaction(transactionId))
  })

  it('Beyond Max values', function () {
    let ret
    // let id = '1';
    const values = new Array(MAX_TAINTED_OBJECTS)
    for (let i = 0; i < values.length; i++) {
      values[i] = i.toString()
      ret = TaintedUtils.newTaintedString(id, values[i], 'param', 'REQUEST')
      assert.strictEqual(ret, values[i], 'Unexpected value')
      ret = TaintedUtils.isTainted(id, ret)
      assert.strictEqual(ret, true, 'Unexpected value')
    }

    // element MAX_TAINTED_OBJECTS + 1
    const beyondLimit = 'beyond'
    ret = TaintedUtils.newTaintedString(id, beyondLimit, 'param', 'REQUEST')
    assert.strictEqual(ret, 'beyond', 'Unexpected value')
    ret = TaintedUtils.isTainted(id, ret)
    assert.strictEqual(ret, false, 'Unexpected value')
  })

  it('One char tainted string must have different instance', function () {
    const oneChar = 'a'
    const taintedOneChar = TaintedUtils.newTaintedString(id, oneChar, 'param', 'request')
    assert.strictEqual(true, TaintedUtils.isTainted(id, taintedOneChar), 'Must be tainted')
    assert.strictEqual(false, TaintedUtils.isTainted(id, oneChar), 'Can not be tainted')
  })
})
