# Repo Compatibility Surface

This document lists every file in this repo that is sensitive to Node.js version
changes and must be checked or updated when adding a new major.

---

## 1. `src/iast_node.h` — V8 API version guard

The only current version guard in the C++ source:

```c
#if defined(NODE_VERSION_v16)
#define IS_EXTERNAL()   IsExternalTwoByte()
#else
#define IS_EXTERNAL()   IsExternal()
#endif
```

**What it protects:** In Node 16, V8 required `IsExternalTwoByte()` to test whether
a string uses an external resource. Node 17 unified this under `IsExternal()`.

**Pattern for new versions:** If a future V8 version renames or removes `IsExternal()`,
a new `#if defined(NODE_VERSION_vX)` branch is needed here. The define is set at
build time via `CMakeLists.txt`.

**Grep to find all guards:**
```sh
grep -rn "NODE_VERSION_v\|IsExternal\|IsExternalTwoByte" src/
```

---

## 2. `CMakeLists.txt` — hardcoded version define

```cmake
add_definitions("-DNODE_VERSION_v18")
```

This define is passed to the C++ compiler and selects which API branch the guards
in `src/iast_node.h` compile against. The `else` branch (no define, or any define
other than `NODE_VERSION_v16`) represents the current default V8 API behavior.

**When to update:** Only when a new V8 API change requires a new branch in
`src/iast_node.h`. If the new Node version is covered by the existing `else` branch,
no change is needed here — the define does not need to track the Node version number.

**How to update:** Add a conditional block or replace with the new version define.
Update `src/iast_node.h` in the same commit.

---

## 3. `.github/workflows/build.yml` — CI version matrices

Every job that runs tests against a Node version matrix must be updated.

Verify the current matrix with:
```sh
grep -A2 "matrix:" .github/workflows/build.yml
```

Example current matrix:
```yaml
matrix:
  version: [18, 19, 20, 21, 22, 23, 24, 25, 26]
```

**Jobs with version matrices (search with `grep -n "version:" .github/workflows/build.yml`):**
- `valgrind` — runs `test:js-valgrind` across all Node versions
- `asan` — runs `test:asan` + `test:js-asan` across all Node versions

**Other version references (not a matrix, but fixed):**
- `js-lint` and `check-licenses` jobs pin `node-version: '20'` — these do not need
  updating when adding a new major.
- `pack` job pins `node-version: '18'` for packaging — does not need updating.

**Action:** Add the new version number to the `version: [...]` arrays in the
`valgrind` and `asan` jobs only.

---

## 4. `package.json` — nan and node-addon-api as header path providers

```json
"nan": "^2.27.0",
"node-addon-api": "^4.3.0"
```

**Neither is an API abstraction layer.** The C++ source uses raw V8 APIs directly
(`#include <node.h>`, `#include <v8.h>`) with no `Nan::` or N-API calls anywhere in
`src/`. These packages are used exclusively to resolve the path to Node.js headers
at build time:

- `binding.gyp` calls `require('nan')` to get the Node headers directory for node-gyp
- `CMakeLists.txt` calls `require('node-addon-api').include` for the CppUTest build

**Compatibility check:** both packages must be able to resolve headers for the new
Node version. If `npm run build` or `npm run test:native` fails with a header
resolution error, follow the pinned upgrade instructions in Step 3 of SKILL.md.
No V8 API compatibility research is needed for these packages themselves.

---

## 5. `binding.gyp` — no version-specific content

`binding.gyp` does not reference Node version numbers. The NAN include resolves
dynamically at build time. No changes expected here unless `nan`'s package
structure changes.
