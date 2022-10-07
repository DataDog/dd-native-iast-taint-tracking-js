const taintedUtilsPkg = process.env.NPM_TAINTEDUTILS === 'true' ? '@datadog/native-iast-taint-tracking' : '../../index'
const TaintedUtils = require(taintedUtilsPkg);

module.exports.TaintedUtils = TaintedUtils