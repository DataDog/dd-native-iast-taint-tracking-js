const fs = require('fs');
const execSync = require('child_process').execSync;
const originalPackageJson = JSON.parse(fs.readFileSync('package.json').toString());
const centos7 = isCentos7(process.platform) ? '-centos7' : '';
const packageJson = {
    name: `@datadog/native-iast-taint-tracking-${process.platform}${centos7}.${process.argv[2]}`,
    version: originalPackageJson.version,
    description: originalPackageJson.description,
    author: originalPackageJson.author,
    license: originalPackageJson.license,
    publishConfig: originalPackageJson.publishConfig
};
fs.writeFileSync('./dist/package.json', JSON.stringify(packageJson));
if(fs.existsSync('.npmrc')) {
    fs.copyFileSync('.npmrc','./dist/.npmrc');
}

function isCentos7(platform) {
    if (platform !== 'linux') {
        return false;
    }
    try {
        const catResult = execSync('cat /etc/os-release').toString();
        const values = catResult.split('\n');
        return values.some(value => value.toLowerCase().includes('centos') || value.toLowerCase().includes('redhat')) &&
            values.some(value => value.toLowerCase().includes('version_id="7'));
    } catch { }
    return false;
}