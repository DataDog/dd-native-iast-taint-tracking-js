/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
'use strict'
const os = require('os')
const path = require('path')
const getLibc = require(path.join(__dirname, 'scripts', 'libc'))
const archFolder = `${os.platform()}-${os.arch()}-${getLibc()}` + '-' + process.versions.node.split('.')[0]
const pathToAddon = path.join('dist', archFolder, 'iastnativemethods.node')

let addon = require(path.join(__dirname, pathToAddon))

let iastNativeMethods = {
    newTaintedString: addon.newTaintedString,
    isTainted: addon.isTainted,
    getRanges : addon.getRanges,
    createTransaction : addon.createTransaction,
    removeTransaction : addon.removeTransaction,
    concat : addon.concat
}

module.exports = iastNativeMethods
