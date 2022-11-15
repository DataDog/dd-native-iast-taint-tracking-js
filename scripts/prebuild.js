const { exec } = require('child_process')
const getLibc = require('./libc')
const GLIBC = 'glibc'
const MUSL = 'musl'

async function prebuild () {
  if (process.platform === 'linux') {
    const libc = getLibc()
    let installPython3
    if (libc === GLIBC) { // build is in centos image
      installPython3 = 'yum -y install python3'
    } else if (libc === MUSL) { // build is in alpine
      installPython3 = 'apk update && apk add python3'
    }
    if (installPython3) {
      return new Promise((resolve, reject) => {
        exec(installPython3, (error, stdout, stderr) => {
          if (error) {
            process.stderr.write(error.message)
            if (stderr) {
              process.stderr.write(stderr)
            }
            reject(error)
          } else {
            process.stdout.write(stdout)
            resolve()
          }
        })
      })
    }
  }
}

prebuild().catch(err => {
  process.stderr.write(JSON.stringify(err))
  process.stdout.write('\n')
})
