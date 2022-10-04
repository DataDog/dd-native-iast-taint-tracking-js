const os = require('os')
const path = require('path')
const getLibc = require('./scripts/libc')
const archFolder = `${os.platform()}-${os.arch()}-${getLibc()}` + '-' + process.versions.node.split('.')[0]
const pathToAddon = path.join('dist', archFolder, 'iastnativemethods.node')

module.exports = require(`./${pathToAddon}`)
