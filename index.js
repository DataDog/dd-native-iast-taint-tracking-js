const os = require('os')
const path = require('path')
const getLibc = require('./scripts/libc')
const archFolder = `${os.platform()}-${os.arch()}-${getLibc()}`
const pathToAddon = path.join('dist', archFolder, 'iastnativemethods.node')

module.exports = require(`./${pathToAddon}`)