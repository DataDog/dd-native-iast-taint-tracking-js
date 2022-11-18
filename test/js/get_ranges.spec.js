/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
const { TaintedUtils } = require('./util')
const assert = require('assert')

describe('Ranges', function () {
  const value = 'test'
  const param = 'p' + 'aram'
  const id = '666'

  afterEach(function () {
    TaintedUtils.removeTransaction(id)
  })

  it('Get Ranges', function () {
    const expected = [{
      start: 0,
      end: value.length,
      iinfo: {
        parameterName: 'param',
        parameterValue: 'test',
        type: 'REQUEST'
      }
    }]
    const taintedValue = TaintedUtils.newTaintedString(id, value, param, 'REQUEST')

    const ranges = TaintedUtils.getRanges(id, taintedValue)
    assert.deepEqual(ranges, expected, 'Ranges expected to be equal')
  })

  it('Wrong number of arguments', function () {
    assert.throws(function () { TaintedUtils.getRanges(id) }, Error)
  })

  it('Get ranges from non tainted string', function () {
    const nonTainted = 'value'

    TaintedUtils.newTaintedString(id, value, param, 'REQUEST')
    const ranges = TaintedUtils.getRanges(id, nonTainted)
    assert.equal(ranges, undefined, 'Ranges expected to be equal')
  })
})
