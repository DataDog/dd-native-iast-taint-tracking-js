const os = require('os')
const path = require('path')
const getLibc = require(path.join(__dirname, '..', 'scripts', 'libc'))
const archFolder = `${os.platform()}-${os.arch()}-${getLibc()}` + '-' + process.versions.node.split('.')[0];
const pathToAddon = path.join(__dirname, '..', 'dist', archFolder, 'iastnativemethods.node')
const addon = require(pathToAddon)

module.exports = addon

