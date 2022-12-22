/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
const taintedUtilsPkg = process.env.NPM_TAINTEDUTILS === 'true' ? '@datadog/native-iast-taint-tracking' : '../../index'
const TaintedUtils = require(taintedUtilsPkg)

const RANGE_OPEN_MARK = ':+-'
const RANGE_CLOSING_MARK = '-+:'

const PARAM_NAME = 'param'
const PARAM_TYPE = 'REQUEST'

function taintFormattedString (transactionId, formattedString) {
  return formattedString
    ? formattedString.split(RANGE_OPEN_MARK).reduce((previousValue, currentValue) => {
      if (currentValue.length === 0) {
        return previousValue
      }
      if (currentValue.indexOf(RANGE_CLOSING_MARK) > -1) {
        const splitParts = currentValue.split(RANGE_CLOSING_MARK)
        const tainted = TaintedUtils.newTaintedString(transactionId, splitParts[0], PARAM_NAME, PARAM_TYPE)
        const previousPlusTainted = TaintedUtils.concat(transactionId, previousValue + tainted, previousValue, tainted)
        if (splitParts.length === 1) return previousPlusTainted
        const literal = splitParts[1]
        return TaintedUtils.concat(transactionId, previousPlusTainted + literal, previousPlusTainted, literal)
      } else {
        return TaintedUtils.concat(transactionId, previousValue + currentValue, previousValue, currentValue)
      }
    }, '')
    : formattedString
}

function formatTaintedValue (transactionId, taintedValue) {
  let offset = 0
  return TaintedUtils.getRanges(transactionId, taintedValue).reduce((formattedString, range) => {
    formattedString =
      formattedString.slice(0, range.start + offset) +
      RANGE_OPEN_MARK +
      formattedString.slice(range.start + offset)
    offset += RANGE_OPEN_MARK.length
    formattedString = formattedString.slice(0, range.end + offset) +
      RANGE_CLOSING_MARK +
      formattedString.slice(range.end + offset)
    offset += RANGE_CLOSING_MARK.length
    return formattedString
  }, taintedValue)
}

module.exports = {
  TaintedUtils,
  taintFormattedString,
  formatTaintedValue
}
