'use strict'
const fs = require('fs')
const path = require('path')
const os = require('os')
const tar = require('tar')
const getLibc = require(path.join(__dirname, 'libc'))
const archFolder = `${os.platform()}-${os.arch()}-${getLibc()}`

fs.mkdirSync(path.join(__dirname, '..', 'dist', archFolder), { recursive: true })
fs.renameSync(path.join(__dirname, '..', 'dist', 'iastnativemethods.node'), path.join(__dirname, '..', 'dist', archFolder, 'iastnativemethods.node'))

tar.c({
    file: path.join(__dirname, '..', `${os.platform()}-${os.arch()}-${getLibc()}.tgz`),
    gzip:true
    },
    [path.join(__dirname,  '..', 'dist')]
)

