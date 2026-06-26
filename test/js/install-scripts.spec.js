/**
* Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
* This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2026 Datadog, Inc.
**/

'use strict'

const assert = require('assert')
const pkg = require('../../package.json')

describe('package manifest', () => {
  it('declares no npm build lifecycle scripts (Yarn Berry YN0007)', () => {
    const scripts = pkg.scripts || {}
    const hooks = ['preinstall', 'install', 'postinstall']
    const present = hooks.filter((name) => scripts[name] !== undefined)
    assert.deepStrictEqual(
      present,
      [],
      'package.json must not declare npm build lifecycle scripts (they trigger Yarn Berry YN0007)'
    )
  })
})
