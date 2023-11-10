# Contributing to dd-native-iast-taint-tracking-js

Please reach out before starting work on any major code changes.
This will ensure we avoid duplicating work, or that your code can't be merged due to a rapidly changing
base. If you would like support for a module that is not listed, [contact support][1] to share a request.

[1]: https://docs.datadoghq.com/help

## Local setup

To set up the project locally, you should either install or build from source `cpputest` in order to execute the tests:

```
apt install libcpputest-dev cpputest
```

and project dependencies:

```
$ npm install
```

Last but not least, you should install `node-gyp` in your environment: [node-gyp installation](https://github.com/nodejs/node-gyp#installation)

Tests can be launched with the following commands:
1. `$ npm run test`
2. `$ npm run test-js`

The first command is used to trigger all the unitary tests of the low level interfaces and the second one run the integration test with node.

### Build

Build the project with

```
$ npm run build
```
