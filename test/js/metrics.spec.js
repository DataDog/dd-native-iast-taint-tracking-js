/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/

'use strict'

const { TaintedUtils } = require('./util')
const assert = require('assert')

describe('Metrics', function () {
  const id = TaintedUtils.createTransaction('1')
  const Verbosity = {
    OFF: 0,
    MANDATORY: 1,
    INFORMATION: 2,
    DEBUG: 3
  }

  afterEach(function () {
    TaintedUtils.removeTransaction(id)
  })

  it('Should return null if transaction does not exist', function () {
    assert.strictEqual(null, TaintedUtils.getMetrics(id, Verbosity.INFORMATION), 'Metrics expected to be null')
  })

  it('Should return null if verbosity level is not a number', function () {
    assert.strictEqual(null, TaintedUtils.getMetrics(id, 'string'), 'Metrics expected to be null')
  })

  it('Should return null if verbosity level is greater than accepted values', function () {
    assert.strictEqual(null, TaintedUtils.getMetrics(id, 4), 'Metrics expected to be null')
  })

  it('Should return the proper request count', function () {
    const expected = {
      requestCount: 1
    }

    TaintedUtils.newTaintedString(id, 'a', 'param', 'request')
    assert.deepEqual(expected, TaintedUtils.getMetrics(id, Verbosity.INFORMATION), 'Metrics expected to be equal')

    TaintedUtils.newTaintedString(id, 'b', 'param', 'request')

    expected.requestCount = 2
    assert.deepEqual(expected, TaintedUtils.getMetrics(id, Verbosity.INFORMATION), 'Metrics expected to be equal')
  })

  it('Should return the properties in all verbosity levels', function () {
    const expected = {
      requestCount: 1
    }

    TaintedUtils.newTaintedString(id, 'a', 'param', 'request')
    assert.deepEqual(expected, TaintedUtils.getMetrics(id, Verbosity.INFORMATION), 'Metrics expected to be equal')
    assert.deepEqual(expected, TaintedUtils.getMetrics(id, Verbosity.DEBUG), 'Metrics expected to be equal')
  })
})
