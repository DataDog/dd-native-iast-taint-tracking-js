/**
 * Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
 * This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
 **/
'use strict'
let addon
try {
  addon = require('node-gyp-build')(__dirname)
} catch (e) {
  addon = {
    createTransaction (transactionId) {
      return transactionId
    },
    newTaintedString (transactionId, original) {
      return original
    },
    isTainted () {
      return false
    },
    getRanges () {
      return undefined
    },
    removeTransaction () {
    },
    concat (transactionId, result) {
      return result
    },
    trim (transaction, result) {
      return result
    },
    trimEnd (transaction, result) {
      return result
    }
  }
}

const iastNativeMethods = {
  newTaintedString: addon.newTaintedString,
  isTainted: addon.isTainted,
  getRanges: addon.getRanges,
  createTransaction: addon.createTransaction,
  removeTransaction: addon.removeTransaction,
  concat: addon.concat,
  trim: addon.trim,
  trimEnd: addon.trimEnd
}

module.exports = iastNativeMethods
