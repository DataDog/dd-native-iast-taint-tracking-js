const semver = require('semver')

const nodeAbiRelation = {
  12: '72',
  13: '79',
  14: '83',
  15: '88',
  16: '93',
  17: '102',
  18: '108',
  19: '111',
  20: '115'
}

function getAbiVersion (version = process.version) {
  return nodeAbiRelation[semver.major(version)]
}

module.exports = getAbiVersion
