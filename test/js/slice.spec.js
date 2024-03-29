/**
 * Unless explicitly stated otherwise all files in this repository are licensed under the Apache-2.0 License.
 * This product includes software developed at Datadog (https://www.datadoghq.com/). Copyright 2022 Datadog, Inc.
 **/
const { TaintedUtils, taintFormattedString, formatTaintedValue } = require('./util')
const assert = require('assert')

describe('Slice', function () {
  const id = '1'

  const testCases = [
    {
      source: ':+-foobarbaz-+:',
      result: '',
      start: 3,
      end: 0
    },
    {
      source: ':+-foobarbaz-+:',
      result: ':+-bar-+:',
      start: -6,
      end: -3
    },
    {
      source: ':+-foobarbaz-+:',
      result: ':+-foo-+:',
      start: 0,
      end: 3
    },
    {
      source: ':+-foobarbaz-+:',
      result: ':+-bar-+:',
      start: -6,
      end: -3
    },
    {
      source: ':+-foobarbaz-+:',
      result: ':+-foo-+:',
      start: -9,
      end: 3
    },
    {
      source: ':+-foobarbaz-+:',
      result: ':+-bar-+:',
      start: 3,
      end: 6
    },
    {
      source: ':+-foobarbaz-+:',
      result: ':+-baz-+:',
      start: 6,
      end: 9
    },
    {
      source: ':+-foobarbaz-+:',
      result: ':+-baz-+:',
      start: 6
    },
    {
      source: 'foo:+-bar-+:baz',
      result: 'foo',
      start: 0,
      end: 3
    },
    {
      source: 'foo:+-bar-+:baz',
      result: 'oo:+-ba-+:',
      start: 1,
      end: 5
    },
    {
      source: 'foo:+-bar-+:baz',
      result: ':+-r-+:baz',
      start: 5,
      end: 9
    },
    {
      source: 'foo:+-bar-+:baz',
      result: 'baz',
      start: 6,
      end: 9
    },
    {
      source: 'foo:+-bar-+:b:+-az-+:',
      result: 'b:+-az-+:',
      start: 6,
      end: 9
    },
    {
      source: 'foo:+-bar-+:b:+-a-+:z',
      result: 'b:+-a-+:z',
      start: 6,
      end: 9
    },
    {
      source: 'foo:+-bar-+:b:+-a-+:z',
      result: 'b:+-a-+:z',
      start: 6,
      end: 15
    },
    {
      source: 'foo:+-bar-+:b:+-a-+:z:+-z-+::+-z-+:',
      result: 'b:+-a-+:z',
      start: 6,
      end: 9
    },
    {
      source: 'foo:+-bar-+:b:+-a-+:z:+-z-+::+-z-+:',
      result: 'b:+-a-+:z:+-z-+:',
      start: 6,
      end: 10
    },
    {
      source: 'foo:+-bar-+:b:+-a-+:z:+-z-+::+-z-+:',
      result: 'b:+-a-+:z:+-z-+::+-z-+:',
      start: 6,
      end: 20
    },
    {
      source: ':+-bar-+:foo:+-baz-+:',
      result: ':+-bar-+:foo:+-b-+:',
      start: -200,
      end: -2
    },
    {
      source: ':+-bar-+:foo:+-baz-+:',
      result: '',
      start: -200,
      end: -30
    },
    {
      source: ':+-bar-+:foo:+-baz-+:',
      result: ':+-bar-+:foo:+-ba-+:',
      start: -200,
      end: 8
    },
    {
      source: ':+-bar-+:foo:+-baz-+:',
      result: ':+-bar-+:foo:+-baz-+:',
      start: -200,
      end: 200
    }
  ]

  afterEach(function () {
    TaintedUtils.removeTransaction(id)
  })

  it('Wrong arguments', function () {
    assert.throws(function () {
      TaintedUtils.slice(id)
    }, Error)
  })

  it('Not tainted', function () {
    const str = 'foobar'

    const res = str.slice(0, 3)
    const ret = TaintedUtils.slice(id, res, str, 0, 3)
    assert.strictEqual(ret, res, 'Unexpected value')
    assert.strictEqual(TaintedUtils.isTainted(id, ret), false, 'Unexpected value')
  })

  it('Tainted value', function () {
    let str = 'foobar'
    str = TaintedUtils.newTaintedString(id, str, 'param', 'REQUEST')

    const res = str.slice(0, 3)
    const ret = TaintedUtils.slice(id, res, str, 0, 3)
    assert.strictEqual(ret, res, 'Unexpected value')
    assert.strictEqual(TaintedUtils.isTainted(id, ret), true, 'Unexpected value')
  })

  it('1 character long', function () {
    let str = 'foobar'
    str = TaintedUtils.newTaintedString(id, str, 'param', 'REQUEST')
    const res = str.slice(0, 1)
    const ret = TaintedUtils.slice(id, res, str, 0, 1)

    assert.strictEqual(ret, res, 'Unexpected value')
    assert.strictEqual(TaintedUtils.isTainted(id, ret), true, 'Unexpected value')
    assert.strictEqual(TaintedUtils.isTainted(id, res), false, 'Unexpected value')
  })

  describe('Range test cases', function () {
    testCases.forEach(({ source, result, start, end }) => {
      it(`Test ${source}`, () => {
        const string = taintFormattedString(id, source)
        assert.equal(TaintedUtils.isTainted(id, string), true, 'String not tainted')
        let res, ret
        if (typeof end === 'undefined') {
          res = string.slice(start)
          ret = TaintedUtils.slice(id, res, string, start)
        } else {
          res = string.slice(start, end)
          ret = TaintedUtils.slice(id, res, string, start, end)
        }

        const formattedResult = formatTaintedValue(id, ret)
        assert.equal(formattedResult, result, 'Unexpected ranges')
      })
    })
  })

  it('Check slice empty string result', function () {
    let op1 = 'hello world'
    op1 = TaintedUtils.newTaintedString(id, op1, 'param1', 'REQUEST')

    let result = op1.slice(6, 6)
    result = TaintedUtils.slice(id, result, op1, 6)

    assert.equal(TaintedUtils.isTainted(id, result), false, 'Empty string is tainted')
  })

  it('Secure marks are inherited', () => {
    let op1 = 'hello world'
    op1 = TaintedUtils.newTaintedString(id, op1, 'param1', 'REQUEST')
    op1 = TaintedUtils.addSecureMarksToTaintedString(id, op1, 0b0110)

    let result = op1.slice(6)
    result = TaintedUtils.slice(id, result, op1, 6)

    const ranges = TaintedUtils.getRanges(id, result)
    assert.equal(ranges.length, 1)
    assert.equal(ranges[0].secureMarks, 0b0110)
  })
})
