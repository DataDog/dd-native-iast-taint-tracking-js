#! /bin/bash

nodeVersions=("v14.15.1" "v16.3.0")
nodeShortVersionNames=("v14" "v16")
initialPath=$PWD
npm install
for index in ${!nodeVersions[*]}
do
    cd $initialPath
    rm -rf ./build
    rm -rf ./dist
    env "NODE_VERSION=${nodeShortVersionNames[$index]}" npm run node-gyp-configure -- --target=${nodeVersions[$index]} && npm run node-gyp-build
    node ./scripts/pre-pack-all.js ${nodeShortVersionNames[$index]} && cd dist && npm pack && cd $initialPath
    cp ./dist/*.tgz .
done
