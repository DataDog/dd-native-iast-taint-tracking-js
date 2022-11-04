/**
 * Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
 * This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
 **/
'use strict'
const proxyquire = require('proxyquire')
const assert = require('assert')
describe('Invalid arch require', function () {
    it('should return an object', () => {
        const TaintedUtils = proxyquire('../../', {
            'os': {
                arch() {
                    return 'invented-arch-does-not-exist'
                }
            }
        })
        assert(TaintedUtils)
        assert(TaintedUtils.createTransaction)
        assert(TaintedUtils.newTaintedString)
        assert(TaintedUtils.isTainted)
        assert(TaintedUtils.getRanges)
        assert(TaintedUtils.removeTransaction)
        assert(TaintedUtils.concat)
    })
})