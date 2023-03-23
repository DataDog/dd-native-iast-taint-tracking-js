/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/

const { TaintedUtils } = require('./util')
const assert = require('assert')

describe('Transaction', function () {
  after(function () {
    TaintedUtils.setMaxTransactions(2)
  })

  it('Create transactions beyond limit with the default value', function () {
    const id = TaintedUtils.createTransaction('1')
    const id2 = TaintedUtils.createTransaction('2')
    const id3 = TaintedUtils.createTransaction('3')

    let ret = TaintedUtils.newTaintedString(id, 'value', 'param', 'REQUEST')
    assert.strictEqual('value', ret)
    assert.strictEqual(true, TaintedUtils.isTainted(id, ret))

    ret = TaintedUtils.newTaintedString(id2, 'value2', 'param', 'REQUEST')
    assert.strictEqual('value2', ret)
    assert.strictEqual(true, TaintedUtils.isTainted(id2, ret))

    ret = TaintedUtils.newTaintedString(id3, 'value3', 'param', 'REQUEST')
    assert.strictEqual('value3', ret)
    assert.strictEqual(false, TaintedUtils.isTainted(id3, ret))

    TaintedUtils.removeTransaction(id)
    TaintedUtils.removeTransaction(id2)
  })

  it('Create transactions beyond limit with a new value', function () {
    TaintedUtils.setMaxTransactions(3)

    const id = TaintedUtils.createTransaction('1')
    const id2 = TaintedUtils.createTransaction('2')
    const id3 = TaintedUtils.createTransaction('3')
    const id4 = TaintedUtils.createTransaction('4')

    let ret = TaintedUtils.newTaintedString(id, 'value', 'param', 'REQUEST')
    assert.strictEqual('value', ret)
    assert.strictEqual(true, TaintedUtils.isTainted(id, ret))

    ret = TaintedUtils.newTaintedString(id2, 'value2', 'param', 'REQUEST')
    assert.strictEqual('value2', ret)
    assert.strictEqual(true, TaintedUtils.isTainted(id2, ret))

    ret = TaintedUtils.newTaintedString(id3, 'value3', 'param', 'REQUEST')
    assert.strictEqual('value3', ret)
    assert.strictEqual(true, TaintedUtils.isTainted(id3, ret))

    ret = TaintedUtils.newTaintedString(id4, 'value4', 'param', 'REQUEST')
    assert.strictEqual('value4', ret)
    assert.strictEqual(false, TaintedUtils.isTainted(id4, ret))

    TaintedUtils.removeTransaction(id)
    TaintedUtils.removeTransaction(id2)
    TaintedUtils.removeTransaction(id3)
  })

  it('Remove transaction beyond the limit', function () {
    TaintedUtils.setMaxTransactions(1)

    const id = TaintedUtils.createTransaction('1')
    const id2 = TaintedUtils.createTransaction('2')

    let ret = TaintedUtils.newTaintedString(id, 'value', 'param', 'REQUEST')
    assert.strictEqual(true, TaintedUtils.isTainted(id, ret))

    ret = TaintedUtils.newTaintedString(id2, 'value2', 'param', 'REQUEST')
    assert.strictEqual(false, TaintedUtils.isTainted(id2, ret))

    TaintedUtils.removeTransaction(id)
    TaintedUtils.removeTransaction(id2)
  })

  it('Remove already deleted transaction', function () {
    TaintedUtils.setMaxTransactions(1)

    const id = TaintedUtils.createTransaction('1')

    const ret = TaintedUtils.newTaintedString(id, 'value', 'param', 'REQUEST')
    assert.strictEqual(true, TaintedUtils.isTainted(id, ret))

    TaintedUtils.removeTransaction(id)
    TaintedUtils.removeTransaction(id)
  })
})
