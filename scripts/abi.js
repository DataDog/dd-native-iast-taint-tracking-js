const semver = require('semver')

const nodeAbiRelation = {
  14: '83',
  15: '88',
  16: '93',
  17: '102',
  18: '108',
  19: '111'
}

function getAbiVersion (version = process.version) {
  return nodeAbiRelation[semver.major(version)]
}

module.exports = getAbiVersion
