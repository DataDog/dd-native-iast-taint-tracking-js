/**
 * Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
 * This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
 **/
'use strict'
const os = require('os')
let addon
try {
    const getLibc = require('./scripts/libc')
    const archFolder = `${os.platform()}-${os.arch()}-${getLibc()}` + '-' + process.versions.node.split('.')[0]
    addon = require(`./dist/${archFolder}/iastnativemethods.node`)
} catch (e) {
    addon = {
        createTransaction(transactionId) {
            return transactionId
        },
        newTaintedString(transactionId, original) {
            return original
        },
        isTainted() {
            return false
        },
        getRanges() {
            return undefined
        },
        removeTransaction() {
        },
        concat(transactionId, result) {
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
    concat: addon.concat
}

module.exports = iastNativeMethods
