/**
 * Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
 * This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
 **/
const { TaintedUtils, taintFormattedString, formatTaintedValue } = require('./util')
const assert = require('assert')

describe('Replace', function () {
  const id = '1'

  afterEach(function () {
    TaintedUtils.removeTransaction(id)
  })

  describe('when matcher and replacer are strings without special char', () => {
    it('One string tainted result is different instance', () => {
      const a = 'a'
      const taintedA = taintFormattedString(id, ':+-a-+:')
      assert.equal(a, taintedA)
      assert.ok(TaintedUtils.isTainted(id, taintedA))
      assert.ok(!TaintedUtils.isTainted(id, a))
      const base = 'ZZZ'
      let result = base.replace('ZZZ', taintedA)
      result = TaintedUtils.replace(id, result, base, 'ZZZ', taintedA)
      assert.equal(result, a)
      assert.ok(TaintedUtils.isTainted(id, taintedA))
      assert.ok(TaintedUtils.isTainted(id, result))
      assert.ok(!TaintedUtils.isTainted(id, a))
    })

    const testCases = [
      {
        description: 'Nothing is tainted',
        self: 'ABC',
        matcher: 'ABC',
        replacer: 'ZYX',
        expected: 'ZYX'
      },
      {
        description: 'Full tainted by full not tainted',
        self: ':+-ABC-+:',
        matcher: 'ABC',
        replacer: 'ZYX',
        expected: 'ZYX'
      },
      {
        description: 'Full tainted by other full tainted',
        self: ':+-ABC-+:',
        matcher: 'ABC',
        replacer: ':+-ZYX-+:',
        expected: ':+-ZYX-+:'
      },
      {
        description: 'Full tainted by other full tainted replace at the beginning',
        self: ':+-ABCDEF-+:',
        matcher: 'ABC',
        replacer: ':+-ZYX-+:',
        expected: ':+-ZYX-+::+-DEF-+:'
      },
      {
        description: 'Full tainted by other full tainted replace in the middle',
        self: ':+-ABCDEF-+:',
        matcher: 'CD',
        replacer: ':+-ZYX-+:',
        expected: ':+-AB-+::+-ZYX-+::+-EF-+:'
      },
      {
        description: 'Full tainted by other full tainted replace in the middle',
        self: ':+-ABCDEF-+:',
        matcher: 'EF',
        replacer: ':+-ZYX-+:',
        expected: ':+-ABCD-+::+-ZYX-+:'
      },
      {
        description: 'Partially tainted string replace not tainted by other not tainted',
        self: 'ABC:+-DEF-+:',
        matcher: 'AB',
        replacer: 'ZYX',
        expected: 'ZYXC:+-DEF-+:'
      },
      {
        description: 'Partially tainted string replace full tainted part by other not tainted',
        self: 'ABC:+-DEF-+:',
        matcher: 'DEF',
        replacer: 'ZYX',
        expected: 'ABCZYX'
      },
      {
        description: 'Partially tainted string replace full tainted part by other tainted',
        self: 'ABC:+-DEF-+:',
        matcher: 'DEF',
        replacer: ':+-ZYX-+:',
        expected: 'ABC:+-ZYX-+:'
      },
      {
        description: 'Partially tainted string replace partially tainted part by other tainted',
        self: 'ABC:+-DEF-+:',
        matcher: 'CD',
        replacer: ':+-ZYX-+:',
        expected: 'AB:+-ZYX-+::+-EF-+:'
      },
      {
        description: 'Partially tainted string replace partially tainted part by not tainted',
        self: 'AB:+-CD-+:EF',
        matcher: 'ABC',
        replacer: 'ZYX',
        expected: 'ZYX:+-D-+:EF'
      },
      {
        description: 'Result is one char length tainted',
        self: 'ABCDEF',
        matcher: 'ABCDEF',
        replacer: ':+-Y-+:',
        expected: ':+-Y-+:'
      },
      {
        description: 'Partially tainted string replace part of the tainted by partially tainted',
        self: ':+-ABC-+:DEF',
        matcher: 'AB',
        replacer: ':+-ZY-+:X:+-WV-+:',
        expected: ':+-ZY-+:X:+-WV-+::+-C-+:DEF'
      },
      {
        description: 'Multiple ranges tainted string replace part of the tainted by partially tainted: 1',
        self: ':+-A-+:B:+-C-+:DEF',
        matcher: 'AB',
        replacer: ':+-ZY-+:X:+-WV-+:',
        expected: ':+-ZY-+:X:+-WV-+::+-C-+:DEF'
      },
      {
        description: 'Multiple ranges tainted string replace part of the tainted by partially tainted: 2',
        self: ':+-A-+:B:+-C-+:D:+-EF-+:',
        matcher: 'AB',
        replacer: ':+-ZY-+:X:+-WV-+:',
        expected: ':+-ZY-+:X:+-WV-+::+-C-+:D:+-EF-+:'
      },
      {
        description: 'Multiple ranges tainted string replace part of the tainted by partially tainted: 3',
        self: ':+-ABC-+:D:+-EF-+:',
        matcher: 'BC',
        replacer: ':+-ZY-+:X:+-WV-+:',
        expected: ':+-A-+::+-ZY-+:X:+-WV-+:D:+-EF-+:'
      },
      {
        description: 'Multiple ranges tainted string full replaced by not tainted string',
        self: ':+-ABC-+:D:+-EF-+:',
        matcher: 'ABCDEF',
        replacer: 'ZYX',
        expected: 'ZYX'
      },
      {
        description: 'Testing with special chars: 1',
        self: ':+-佫𝒳-+: :+-😂😂😂-+: :+-𝒳-+:',
        matcher: '佫',
        replacer: ':+-𝒳-+:',
        expected: ':+-𝒳-+::+-𝒳-+: :+-😂😂😂-+: :+-𝒳-+:'
      },
      {
        description: 'Testing with special chars: 2',
        self: ':+-佫𝒳-+: :+-😂😂😂-+: :+-𝒳-+:',
        matcher: '😂😂😂',
        replacer: '🥲🥲🥲',
        expected: ':+-佫𝒳-+: 🥲🥲🥲 :+-𝒳-+:'
      },
      {
        description: 'Ignore special replacer and not taint result: 1',
        self: 'za',
        matcher: 'a',
        replacer: 'a$`0`',
        expected: 'zaz0`'
      },
      {
        description: 'Ignore special replacer and not taint result: 2',
        self: 'za',
        matcher: 'a',
        replacer: '$&$&$&',
        expected: 'zaaa'
      },
      {
        description: 'Only the replacer is tainted',
        self: 'ABCDEF',
        matcher: 'CD',
        replacer: ':+-ZYX-+:',
        expected: 'AB:+-ZYX-+:EF'
      }
    ]

    testCases.forEach(testCase => {
      it(testCase.description, () => {
        const self = taintFormattedString(id, testCase.self)
        const replacer = taintFormattedString(id, testCase.replacer)
        const { matcher, expected } = testCase
        let result = self.replace(matcher, replacer)
        result = TaintedUtils.replace(id, result, self, matcher, replacer)
        assert.equal(formatTaintedValue(id, result), expected, 'Unexpected vale')
      })
    })
  })

  describe('when the matcher is a Regex and replacer is string without special char', () => {
    it('One string tainted result is different instance', () => {
      const a = 'a'
      const taintedA = taintFormattedString(id, ':+-a-+:')
      assert.equal(a, taintedA)
      assert.ok(TaintedUtils.isTainted(id, taintedA))
      assert.ok(!TaintedUtils.isTainted(id, a))
      const base = 'ZZZ'
      let result = base.replace(/ZZZ/, taintedA)
      result = TaintedUtils.replace(id, result, base, 'ZZZ', taintedA)
      assert.equal(result, a)
      assert.ok(TaintedUtils.isTainted(id, taintedA))
      assert.ok(TaintedUtils.isTainted(id, result))
      assert.ok(!TaintedUtils.isTainted(id, a))
    })

    const testCases = [
      {
        description: 'Nothing is tainted',
        self: 'ABC',
        matcher: /ABC/,
        replacer: 'ZYX',
        expected: 'ZYX'
      },
      {
        description: 'Full tainted by full not tainted',
        self: ':+-ABC-+:',
        matcher: /ABC/,
        replacer: 'ZYX',
        expected: 'ZYX'
      },
      {
        description: 'Full tainted by other full tainted',
        self: ':+-ABC-+:',
        matcher: /ABC/,
        replacer: ':+-ZYX-+:',
        expected: ':+-ZYX-+:'
      },
      {
        description: 'Full tainted by other full tainted replace at the beginning',
        self: ':+-ABCDEF-+:',
        matcher: /ABC/,
        replacer: ':+-ZYX-+:',
        expected: ':+-ZYX-+::+-DEF-+:'
      },
      {
        description: 'Full tainted by other full tainted replace in the middle',
        self: ':+-ABCDEF-+:',
        matcher: /CD/,
        replacer: ':+-ZYX-+:',
        expected: ':+-AB-+::+-ZYX-+::+-EF-+:'
      },
      {
        description: 'Full tainted by other full tainted replace in the middle',
        self: ':+-ABCDEF-+:',
        matcher: /EF/,
        replacer: ':+-ZYX-+:',
        expected: ':+-ABCD-+::+-ZYX-+:'
      },
      {
        description: 'Partially tainted string replace not tainted by other not tainted',
        self: 'ABC:+-DEF-+:',
        matcher: /AB/,
        replacer: 'ZYX',
        expected: 'ZYXC:+-DEF-+:'
      },
      {
        description: 'Partially tainted string replace full tainted part by other not tainted',
        self: 'ABC:+-DEF-+:',
        matcher: /DEF/,
        replacer: 'ZYX',
        expected: 'ABCZYX'
      },
      {
        description: 'Partially tainted string replace full tainted part by other tainted',
        self: 'ABC:+-DEF-+:',
        matcher: /DEF/,
        replacer: ':+-ZYX-+:',
        expected: 'ABC:+-ZYX-+:'
      },
      {
        description: 'Partially tainted string replace partially tainted part by other tainted',
        self: 'ABC:+-DEF-+:',
        matcher: /CD/,
        replacer: ':+-ZYX-+:',
        expected: 'AB:+-ZYX-+::+-EF-+:'
      },
      {
        description: 'Partially tainted string replace partially tainted part by not tainted',
        self: 'AB:+-CD-+:EF',
        matcher: /ABC/,
        replacer: 'ZYX',
        expected: 'ZYX:+-D-+:EF'
      },
      {
        description: 'Result is one char length tainted',
        self: 'ABCDEF',
        matcher: /ABCDEF/,
        replacer: ':+-Y-+:',
        expected: ':+-Y-+:'
      },
      {
        description: 'Partially tainted string replace part of the tainted by partially tainted',
        self: ':+-ABC-+:DEF',
        matcher: /AB/,
        replacer: ':+-ZY-+:X:+-WV-+:',
        expected: ':+-ZY-+:X:+-WV-+::+-C-+:DEF'
      },
      {
        description: 'Multiple ranges tainted string replace part of the tainted by partially tainted: 1',
        self: ':+-A-+:B:+-C-+:DEF',
        matcher: /AB/,
        replacer: ':+-ZY-+:X:+-WV-+:',
        expected: ':+-ZY-+:X:+-WV-+::+-C-+:DEF'
      },
      {
        description: 'Multiple ranges tainted string replace part of the tainted by partially tainted: 2',
        self: ':+-A-+:B:+-C-+:D:+-EF-+:',
        matcher: /AB/,
        replacer: ':+-ZY-+:X:+-WV-+:',
        expected: ':+-ZY-+:X:+-WV-+::+-C-+:D:+-EF-+:'
      },
      {
        description: 'Multiple ranges tainted string replace part of the tainted by partially tainted: 3',
        self: ':+-ABC-+:D:+-EF-+:',
        matcher: /BC/,
        replacer: ':+-ZY-+:X:+-WV-+:',
        expected: ':+-A-+::+-ZY-+:X:+-WV-+:D:+-EF-+:'
      },
      {
        description: 'Multiple ranges tainted string full replaced by not tainted string',
        self: ':+-ABC-+:D:+-EF-+:',
        matcher: /ABCDEF/,
        replacer: 'ZYX',
        expected: 'ZYX'
      },
      {
        description: 'Testing with special chars: 1',
        self: ':+-佫𝒳-+: :+-😂😂😂-+: :+-𝒳-+:',
        matcher: /佫/,
        replacer: ':+-𝒳-+:',
        expected: ':+-𝒳-+::+-𝒳-+: :+-😂😂😂-+: :+-𝒳-+:'
      },
      {
        description: 'Testing with special chars: 2',
        self: ':+-佫𝒳-+: :+-😂😂😂-+: :+-𝒳-+:',
        matcher: /😂😂😂/,
        replacer: '🥲🥲🥲',
        expected: ':+-佫𝒳-+: 🥲🥲🥲 :+-𝒳-+:'
      },
      {
        description: 'Ignore special replacer and not taint result: 1',
        self: 'za',
        matcher: /a/,
        replacer: 'a$`0`',
        expected: 'zaz0`'
      },
      {
        description: 'Ignore special replacer and not taint result: 2',
        self: 'za',
        matcher: /a/,
        replacer: '$&$&$&',
        expected: 'zaaa'
      },
      {
        description: 'Only the replacer is tainted',
        self: 'ABCDEF',
        matcher: /CD/,
        replacer: ':+-ZYX-+:',
        expected: 'AB:+-ZYX-+:EF'
      },
      {
        description: 'Multiple replaces in not tainted string by fully tainted string',
        self: 'ABABAB',
        matcher: /B/g,
        replacer: ':+-ZYX-+:',
        expected: 'A:+-ZYX-+:A:+-ZYX-+:A:+-ZYX-+:'
      },
      {
        description: 'Multiple replaces in not tainted string by partially tainted string',
        self: 'ABABAB',
        matcher: /B/g,
        replacer: ':+-Z-+:YX',
        expected: 'A:+-Z-+:YXA:+-Z-+:YXA:+-Z-+:YX'
      },
      {
        description: 'Multiple replaces in not tainted string by multi tainted string',
        self: 'ABABAB',
        matcher: /B/g,
        replacer: ':+-Z-+:Y:+-X-+:',
        expected: 'A:+-Z-+:Y:+-X-+:A:+-Z-+:Y:+-X-+:A:+-Z-+:Y:+-X-+:'
      },
      {
        description: 'Multiple replaces in fully tainted string by fully tainted string',
        self: ':+-ABABAB-+:',
        matcher: /B/g,
        replacer: ':+-ZYX-+:',
        expected: ':+-A-+::+-ZYX-+::+-A-+::+-ZYX-+::+-A-+::+-ZYX-+:'
      },
      {
        description: 'Multiple replaces in fully tainted string by partially tainted string',
        self: ':+-ABABAB-+:',
        matcher: /B/g,
        replacer: ':+-Z-+:YX',
        expected: ':+-A-+::+-Z-+:YX:+-A-+::+-Z-+:YX:+-A-+::+-Z-+:YX'
      },
      {
        description: 'Multiple replaces in fully tainted string by multi tainted string',
        self: ':+-ABABAB-+:',
        matcher: /B/g,
        replacer: ':+-Z-+:Y:+-X-+:',
        expected: ':+-A-+::+-Z-+:Y:+-X-+::+-A-+::+-Z-+:Y:+-X-+::+-A-+::+-Z-+:Y:+-X-+:'
      },
      {
        description: 'Multiple replaces in multi tainted string by fully tainted string',
        self: ':+-ABA-+:JJ:+-BAB-+:',
        matcher: /BA/g,
        replacer: ':+-ZYX-+:',
        expected: ':+-A-+::+-ZYX-+:JJ:+-ZYX-+::+-B-+:'
      },
      {
        description: 'Multiple replaces in multi tainted string by partially tainted string',
        self: ':+-ABA-+:JJ:+-BAB-+:',
        matcher: /BA/g,
        replacer: ':+-Z-+:YX',
        expected: ':+-A-+::+-Z-+:YXJJ:+-Z-+:YX:+-B-+:'
      },
      {
        description: 'Multiple replaces in multi tainted string by multi tainted string',
        self: ':+-ABA-+:JJ:+-BAB-+:',
        matcher: /BA/g,
        replacer: ':+-Z-+:Y:+-X-+:',
        expected: ':+-A-+::+-Z-+:Y:+-X-+:JJ:+-Z-+:Y:+-X-+::+-B-+:'
      },
      {
        description: 'Multiple replaces in multi tainted string by multi tainted string replacing semi-tainted',
        self: ':+-ABA-+:JJ:+-BABA-+:JJ',
        matcher: /AJ/g,
        replacer: ':+-Z-+:Y:+-X-+:',
        expected: ':+-AB-+::+-Z-+:Y:+-X-+:J:+-BAB-+::+-Z-+:Y:+-X-+:J'
      },
      {
        description: 'Multiple replaces in multi tainted string by multi tainted string replacing ' +
            'semi-tainted with group in regex',
        self: ':+-ABA-+:JJ:+-BABA-+:JJ',
        matcher: /A(J)/g,
        replacer: ':+-Z-+:Y:+-X-+:',
        expected: ':+-AB-+::+-Z-+:Y:+-X-+:J:+-BAB-+::+-Z-+:Y:+-X-+:J'
      }
    ]

    testCases.forEach(testCase => {
      it(testCase.description, () => {
        const self = taintFormattedString(id, testCase.self)
        const replacer = taintFormattedString(id, testCase.replacer)
        const { matcher, expected } = testCase
        let result = self.replace(matcher, replacer)
        result = TaintedUtils.replace(id, result, self, matcher, replacer)
        assert.equal(formatTaintedValue(id, result), expected, 'Unexpected vale')
      })
    })
  })
})
