---
name: check-node-compat
description: |
  Check compatibility with a new Node.js major version for this native C++ addon.
  Use when a new Node.js major is released and needs to be added to the supported
  matrix, or when asked to verify whether the addon works on a specific Node version.
  Triggers: "add Node X support", "does this work on Node X", "new Node major",
  "update Node version matrix", "Node compatibility".
---

# Node.js Compatibility Check Skill

## Goal

Determine what changes are needed to support a new Node.js major version in this
native C++ addon. The addon uses raw V8 APIs directly — there is no abstraction
layer shielding it from breaking changes. `nan` and `node-addon-api` are present
only as header path providers for the build system, not as API wrappers.

This means the V8 API scan is the highest-risk step. Everything else is mechanical.

The output is a concrete report: what is safe, what needs a mechanical change, and
what needs manual verification or a new CI run.

## Step 1 — Identify the new V8 version

Each Node.js major embeds a specific V8 version. Find it with a web search:

```
Node.js X.0.0 release notes V8 version
```

Or check the official Node.js changelog at `https://nodejs.org/en/blog/release/`.
Record the V8 version (e.g., `V8 13.0`). This is the input to Step 2.

See [references/research-sources.md](references/research-sources.md) for additional
sources to find the exact V8 version embedded in a given Node release.

## Step 2 — Scan for raw V8 API changes (highest risk)

The source uses V8 APIs directly with no abstraction layer. Any V8 API rename,
removal, or signature change in the new version is a direct build break.

**2a. Find existing version guards:**
```sh
grep -rn "NODE_VERSION_v" src/
```

For each guard found, check whether the API it wraps has changed in the new V8
version. See [references/repo-compat-surface.md](references/repo-compat-surface.md)
for the current guards and what they protect.

**2b. Find direct V8 API calls most likely to change between versions:**
```sh
# V8 String API (historically the most volatile area in this codebase)
grep -rn "v8::String::\|->IsExternal\|->IsString\|ExternalString\|NewFromUtf8\|Utf8Value" src/

# V8 Isolate and context API
grep -rn "GetIsolate\(\)\|GetCurrentContext\(\)\|isolate->Get\|isolate->Throw" src/
```

These are the V8 API families that have changed most frequently across Node majors.
Cross-reference hits against the V8 API diff for the old→new version:
`https://github.com/v8/v8/compare/OLD.V-lkgr...NEW.V-lkgr`
Focus on changes under `include/` — those are the public API surface.

Web search template for a specific function:
```
V8 API "<function_name>" deprecated OR removed OR changed vNEW
```

See [references/research-sources.md](references/research-sources.md) for V8 diff
and changelog sources.

## Step 3 — Check header path providers

`nan` and `node-addon-api` are not API abstraction layers in this codebase — they
are used only to resolve the path to Node.js headers at build time:

- `binding.gyp` uses `require('nan')` to locate Node headers for node-gyp
- `CMakeLists.txt` uses `require('node-addon-api').include` for the CppUTest build

The check here is minimal: activate the new Node version (e.g. `nvm use X`), then
run both build steps to confirm headers resolve correctly:

```sh
npm run build          # node-gyp build — uses nan for headers
npm run test:native    # CppUTest build — uses node-addon-api for headers
```

If CppUTest is not installed locally, skip `npm run test:native` and rely on CI
to surface any header resolution failure for that path.

No API compatibility research is needed for these packages.

If either command fails with a header resolution error, upgrade the offending
package to a pinned stable version:

```sh
# Find the latest stable version
npm view nan version
npm view node-addon-api version

# Pin the version explicitly in package.json, then install
npm install nan@X.Y.Z
# or
npm install node-addon-api@X.Y.Z
```

Do not use `@latest` — versions must be pinned.

## Step 4 — Update the hardcoded version define in CMakeLists.txt

[CMakeLists.txt](../../../CMakeLists.txt) contains `-DNODE_VERSION_v18`, which
drives the `#if defined(...)` guards found in Step 2.

- If Step 2 found no new API changes: no update needed — the current `else` branch
  already covers the new version.
- If Step 2 found a new API change requiring a new branch: add
  `#if defined(NODE_VERSION_vX)` in `src/iast_node.h` and update the define in
  `CMakeLists.txt`. Always update both files in the same commit.

**Cleaning up obsolete guards:** if the lowest version in the CI matrix has advanced
past the version a guard was written for (e.g., `NODE_VERSION_v16` once Node 16 is
removed from the matrix), the guard and its older branch are dead code. Remove them
and simplify the macro to the surviving branch. Do this in a dedicated cleanup commit,
not mixed with the new version addition.

See [references/repo-compat-surface.md](references/repo-compat-surface.md) for the
full picture of this define and the current guards.

## Step 5 — Update the CI version matrix

Add the new Node version to the test matrices in
[.github/workflows/build.yml](../../../.github/workflows/build.yml).

See [references/repo-compat-surface.md](references/repo-compat-surface.md) for
which specific jobs have version matrices and which have fixed pinned versions.

## Step 6 — Produce the compatibility report

```
## Node X Compatibility Report

### V8 API scan
- Guards found: <list from grep, or "none">
- Unguarded V8 calls reviewed: <count reviewed>
- Status per guard/call: <safe / needs new branch / unknown>
  Reason: <what changed in V8 X.Y, or "no change found in changelog">

### Obsolete guards
- <guard name>: obsolete since Node <Y> was removed from the matrix — <action: remove / keep>
- (or: none detected)

### Build system (nan / node-addon-api)
- Header resolution: OK / FAILED (upgrade needed)
- Action: none / upgrade <package> to X.Y.Z

### CMakeLists.txt
- NODE_VERSION define: <current> — <action: no change / update to NODE_VERSION_vX>

### CI Matrix (.github/workflows/build.yml)
- Jobs updated: <list>

### Verdict
SAFE TO ADD / NEEDS CHANGES / REQUIRES MANUAL VERIFICATION
```

Mark REQUIRES MANUAL VERIFICATION if any V8 API call could not be confirmed safe
from changelog research alone. State what the CI run will surface.

## What this skill cannot guarantee

The definitive answer comes from running CI against the new Node version. Because
the code uses raw V8 APIs, a breaking change can exist without any changelog entry
if it was an undocumented internal. The green CI run on the new version is the
final sign-off.
