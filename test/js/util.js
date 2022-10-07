/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
**/
const taintedUtilsPkg = process.env.NPM_TAINTEDUTILS === 'true' ? '@datadog/native-iast-taint-tracking' : '../../index'
const TaintedUtils = require(taintedUtilsPkg);

module.exports.TaintedUtils = TaintedUtils
