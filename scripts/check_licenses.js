'use strict'

const fs = require('fs')
const path = require('path')
const pkg = require('../package.json')

const filePath = path.join(__dirname, '..', '/LICENSE-3rdparty.csv')
const thirdPartyCsv = fs.readFileSync(filePath).toString()
const csvLines = thirdPartyCsv.split('\n')
const deps = new Set(Object.keys(pkg.dependencies || {}))
const devDeps = new Set(Object.keys(pkg.devDependencies || {}))

// I don't know how to check c++ libraries
// For the moment, we have to add it manually
devDeps.add('cpputest')

let index = 0

const licenses = {
  require: new Set(),
  dev: new Set(),
  file: new Set()
}

/* eslint-disable no-console */
for (let i = 0; i < csvLines.length; i++) {
  const line = csvLines[i]
  if (index !== 0) {
    const columns = line.split(',')
    const type = columns[0]
    const license = columns[1]
    if (columns.length > 4) {
      console.log(`Too many columns in line: ${line}`)
    }	  
    if (type && license) {
      licenses[type].add(license)
    }
  }

  index++
}
const requiresOk = checkLicenses(deps, 'require')
const devOk = checkLicenses(devDeps, 'dev')
if (!requiresOk || !devOk) {
  process.exit(1)
}

function checkLicenses (typeDeps, type) {
  const missing = []
  const extraneous = []

  for (const dep of typeDeps) {
    if (!licenses[type].has(dep)) {
      missing.push(dep)
    }
  }

  for (const dep of licenses[type]) {
    if (!typeDeps.has(dep)) {
      extraneous.push(dep)
    }
  }

  if (missing.length) {
    console.log(`Missing ${type} 3rd-party license for ${missing.join(', ')}.`)
  }

  if (extraneous.length) {
    console.log(`Extraneous ${type} 3rd-party license for ${extraneous.join(', ')}.`)
  }

  return missing.length === 0 && extraneous.length === 0
}
