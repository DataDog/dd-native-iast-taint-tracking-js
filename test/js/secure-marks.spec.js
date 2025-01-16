/**
 * Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
 * This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
 **/
'use strict'

const { TaintedUtils } = require('./util')
const assert = require('assert')

let referenceEqual
function makeReferenceEqual () {
  // eslint-disable-next-line no-new-func
  referenceEqual = new Function('value1', 'value2', 'return %ReferenceEqual(value1, value2)')
}

try {
  makeReferenceEqual()
} catch (e) {
  try {
    const v8 = require('v8')
    v8.setFlagsFromString('--allow-natives-syntax')
    makeReferenceEqual()
    v8.setFlagsFromString('--no-allow-natives-syntax')
  // eslint-disable-next-line no-empty
  } catch (e) {/* empty */}
}

describe('Secure marks', function () {
  const id = TaintedUtils.createTransaction('666')
  const value = 'test'
  const param = 'param'

  afterEach(function () {
    TaintedUtils.removeTransaction(id)
  })

  it('Wrong arguments', function () {
    assert.throws(function () {
      TaintedUtils.addSecureMarksToTaintedString(id, value)
    }, Error)
  })

  it('By default securemarks is 0', () => {
    const taintedValue = TaintedUtils.newTaintedString(id, value, param, 'REQUEST')
    const ranges = TaintedUtils.getRanges(id, taintedValue)

    assert.equal(ranges.length, 1)
    assert.equal(ranges[0].secureMarks, 0)
  })

  it('Add secure marks does not modify original ranges', () => {
    const originalTaintedValue = TaintedUtils.newTaintedString(id, value, param, 'REQUEST')
    const taintedValueWithSecureMarks = TaintedUtils.addSecureMarksToTaintedString(id, originalTaintedValue, 0b1)
    const oRanges = TaintedUtils.getRanges(id, originalTaintedValue)
    const markedRanges = TaintedUtils.getRanges(id, taintedValueWithSecureMarks)

    assert.equal(markedRanges.length, 0b1)
    assert.equal(markedRanges[0].secureMarks, 1)

    assert.equal(oRanges.length, 1)
    assert.equal(oRanges[0].secureMarks, 0)
  })

  it('Multiple secure marks', () => {
    const originalTaintedValue = TaintedUtils.newTaintedString(id, value, param, 'REQUEST')

    const taintedValueWithSecureMarks1 = TaintedUtils.addSecureMarksToTaintedString(id, originalTaintedValue, 0b001)
    const taintedValueWithSecureMarks2 = TaintedUtils.addSecureMarksToTaintedString(id,
      taintedValueWithSecureMarks1, 0b100)
    const taintedValueWithSecureMarks3 = TaintedUtils.addSecureMarksToTaintedString(id,
      taintedValueWithSecureMarks2, 0b010)

    const oRanges = TaintedUtils.getRanges(id, originalTaintedValue)
    const markedRanges1 = TaintedUtils.getRanges(id, taintedValueWithSecureMarks1)
    const markedRanges2 = TaintedUtils.getRanges(id, taintedValueWithSecureMarks2)
    const markedRanges3 = TaintedUtils.getRanges(id, taintedValueWithSecureMarks3)

    assert.equal(markedRanges1.length, 1)
    assert.equal(markedRanges1[0].secureMarks, 0b1)

    assert.equal(markedRanges2.length, 1)
    assert.equal(markedRanges2[0].secureMarks, 0b101)

    assert.equal(markedRanges3.length, 1)
    assert.equal(markedRanges3[0].secureMarks, 0b111)

    assert.equal(oRanges.length, 1)
    assert.equal(oRanges[0].secureMarks, 0)
  })

  it('Secure marks apply to all ranges', () => {
    const taintedValue1 = TaintedUtils.newTaintedString(id, 'firstRange', param, 'REQUEST')
    const taintedValue2 = TaintedUtils.newTaintedString(id, 'secondRange', param, 'REQUEST')
    let originalTaintedValue = TaintedUtils.concat(id, taintedValue1 + ' ', taintedValue1, ' ')
    originalTaintedValue = TaintedUtils.concat(id, originalTaintedValue + taintedValue2,
      originalTaintedValue, taintedValue2)
    assert.equal(originalTaintedValue, 'firstRange secondRange')

    const taintedValueWithSecureMarks1 = TaintedUtils.addSecureMarksToTaintedString(id, originalTaintedValue, 0b001)
    const taintedValueWithSecureMarks2 = TaintedUtils.addSecureMarksToTaintedString(id,
      taintedValueWithSecureMarks1, 0b100)

    const oRanges = TaintedUtils.getRanges(id, originalTaintedValue)
    const markedRanges1 = TaintedUtils.getRanges(id, taintedValueWithSecureMarks1)
    const markedRanges2 = TaintedUtils.getRanges(id, taintedValueWithSecureMarks2)

    assert.equal(markedRanges1.length, 2)
    assert.equal(markedRanges1[1].secureMarks, 0b1)
    assert.equal(markedRanges1[1].secureMarks, 0b1)

    assert.equal(markedRanges2.length, 2)
    assert.equal(markedRanges2[1].secureMarks, 0b101)
    assert.equal(markedRanges2[1].secureMarks, 0b101)

    assert.equal(oRanges.length, 2)
    assert.equal(oRanges[0].secureMarks, 0)
    assert.equal(oRanges[1].secureMarks, 0)
  })

  it('Secure marks are inherited in string modifications', () => {
    const originalTaintedValue = TaintedUtils.newTaintedString(id, 'range1TOREPLACErange2', param, 'REQUEST')
    const taintedValueWithSecureMarks = TaintedUtils.addSecureMarksToTaintedString(id, originalTaintedValue, 0b1)

    let result = taintedValueWithSecureMarks.replace('TOREPLACE', ' ')
    result = TaintedUtils.replace(id, result, taintedValueWithSecureMarks, 'TOREPLACE', ' ')
    assert.equal(result, 'range1 range2')

    const markedRanges1 = TaintedUtils.getRanges(id, result)

    assert.equal(markedRanges1.length, 2)
    assert.equal(markedRanges1[0].secureMarks, 0b1)
    assert.equal(markedRanges1[1].secureMarks, 0b1)
  })

  it('Secure marks limits test', () => {
    const originalTaintedValue = TaintedUtils.newTaintedString(id, 'range1TOREPLACErange2', param, 'REQUEST')
    const taintedValueWithSecureMarks = TaintedUtils.addSecureMarksToTaintedString(id, originalTaintedValue, 0xffaf)
    const taintedValueWithSecureMarks2 =
      TaintedUtils.addSecureMarksToTaintedString(id, originalTaintedValue, 0x1ffffffff) // over the limit

    const markedRanges1 = TaintedUtils.getRanges(id, taintedValueWithSecureMarks)
    const markedRanges2 = TaintedUtils.getRanges(id, taintedValueWithSecureMarks2)
    assert.equal(markedRanges1[0].secureMarks, 0xffaf)
    assert.equal(markedRanges2[0].secureMarks, 0xffffffff)
  })

  describe('createNewTainted flag', () => {
    it('when false no new tainted should be created', () => {
      const originalTaintedValue = TaintedUtils.newTaintedString(id, 'tainted', param, 'REQUEST')
      const taintedWithSecureMarks = TaintedUtils.addSecureMarksToTaintedString(id, originalTaintedValue, 0b1, false)

      assert(referenceEqual(originalTaintedValue, taintedWithSecureMarks))

      const markedRanges1 = TaintedUtils.getRanges(id, originalTaintedValue)
      const markedRanges2 = TaintedUtils.getRanges(id, taintedWithSecureMarks)
      assert.equal(markedRanges1[0].secureMarks, 0b1)
      assert.equal(markedRanges2[0].secureMarks, 0b1)
    })

    it('when undefined new tainted should be created', () => {
      const originalTaintedValue = TaintedUtils.newTaintedString(id, 'tainted', param, 'REQUEST')
      const taintedWithSecureMarks = TaintedUtils.addSecureMarksToTaintedString(id, originalTaintedValue, 0b1)

      const markedRanges1 = TaintedUtils.getRanges(id, originalTaintedValue)
      const markedRanges2 = TaintedUtils.getRanges(id, taintedWithSecureMarks)
      assert.equal(markedRanges1[0].secureMarks, 0)
      assert.equal(markedRanges2[0].secureMarks, 0b1)

      assert(!referenceEqual(originalTaintedValue, taintedWithSecureMarks))
    })

    it('when true new tainted should be created', () => {
      const originalTaintedValue = TaintedUtils.newTaintedString(id, 'tainted', param, 'REQUEST')
      const taintedWithSecureMarks = TaintedUtils.addSecureMarksToTaintedString(id, originalTaintedValue, 0b1, true)

      const markedRanges1 = TaintedUtils.getRanges(id, originalTaintedValue)
      const markedRanges2 = TaintedUtils.getRanges(id, taintedWithSecureMarks)
      assert.equal(markedRanges1[0].secureMarks, 0)
      assert.equal(markedRanges2[0].secureMarks, 0b1)

      assert(!referenceEqual(originalTaintedValue, taintedWithSecureMarks))
    })
  })
})
