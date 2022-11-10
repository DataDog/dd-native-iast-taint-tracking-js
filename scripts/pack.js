'use strict'
const fs = require('fs')
const os = require('os')
const path = require('path')
const getLibc = require('./libc')
const getAbiVersion = require('./abi')

const TARGET_NAME = 'iastnativemethods'
const platform = os.platform()
const arch = process.env.ARCH || os.arch()
const libc = getLibc() === 'musl' ? 'musl' : ''

const outputdir = path.join('prebuilds', `${platform}${libc}-${arch}`)
const output = path.join(outputdir, `/node-${getAbiVersion()}.node`)
fs.mkdirSync(`prebuilds/${platform}${libc}-${arch}`, { recursive: true })
fs.copyFileSync(`build/Release/${TARGET_NAME}.node`, output)
