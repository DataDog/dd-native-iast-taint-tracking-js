'use strict'
const fs = require('fs')
const path = require('path')
const childProcess = require('child_process')
const os = require('os')
const tar = require('tar')

function getLibc() {
    if (process.platform === 'linux') {
        const out = childProcess.execSync('ldd --version 2>&1 || true')
        if (out.includes('GLIBC')) {
            return 'glibc'
        } else if (out.includes('musl')) {
            return 'musl'
        } else {
            return 'unknown'
        }
    }
    else {
        return 'unknown'
    }
}

const archFolder = `${os.platform()}-${os.arch()}-${getLibc()}`

fs.mkdirSync(path.join(__dirname, '..', 'dist', archFolder), { recursive: true })
fs.renameSync(path.join(__dirname, '..', 'dist', 'iastnativemethods.node'), path.join(__dirname, '..', 'dist', archFolder, 'iastnativemethods.node'))

tar.c({
    file: path.join(__dirname, '..', `${os.platform()}-${os.arch()}-${getLibc()}.tgz`),
    gzip:true
    },
    [path.join(__dirname,  '..', 'dist')]
)

