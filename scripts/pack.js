'use strict'
const fs = require('fs')
const path = require('path')
const os = require('os')
const getLibc = require(path.join(__dirname, 'libc'))

let archFolder = `${os.platform()}-${os.arch()}-${getLibc()}`

archFolder += '-' + process.versions.node.split('.')[0]

fs.mkdirSync(path.join(__dirname, '..', 'dist', archFolder), { recursive: true })
fs.renameSync(path.join(__dirname, '..', 'dist', 'iastnativemethods.node'), path.join(__dirname, '..', 'dist', archFolder, 'iastnativemethods.node'))
