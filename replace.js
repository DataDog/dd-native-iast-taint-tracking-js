const isSpecialRegex = /(\$\$)|(\$&)|(\$`)|(\$')|(\$\d)/
function getReplace (addon) {
  if (addon.replace) {
    return addon.replace
  }
  return function replace (transactionId, result, thisArg, matcher, replacer) {
    if (transactionId && typeof thisArg === 'string') {
      if (typeof matcher === 'string' && typeof replacer === 'string') {
        if (addon.isTainted(transactionId, thisArg, replacer)) {
          const isSpecialReplacement = replacer.indexOf('$') > -1 && !!replacer.match(isSpecialRegex)
          if (!isSpecialReplacement) {
            const index = thisArg.indexOf(matcher)
            if (index > -1) {
              return addon.replaceStringByString(transactionId, result, thisArg, matcher, replacer, index)
            }
          }
        }
      }
    }
    return result
  }
}

module.exports = getReplace
