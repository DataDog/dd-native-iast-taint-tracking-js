# Research Sources for Node.js Compatibility

Quick reference for finding V8 compatibility information. Note: `nan` and
`node-addon-api` are used only as header path providers in this repo — they are
not API abstraction layers. V8 API research is the primary concern.

---

## Node.js release information

**Official release blog** (V8 version per Node release):
`https://nodejs.org/en/blog/release/vX.0.0`
Look for the line: *"V8 X.Y"* in the highlights section.

**Node.js changelog on GitHub:**
`https://github.com/nodejs/node/blob/main/CHANGELOG.md`
Filter by the target major version tag.

**Node.js deps/v8 directory** (exact V8 version embedded):
`https://github.com/nodejs/node/tree/vX.0.0/deps/v8`
The `include/v8-version.h` file contains `V8_MAJOR_VERSION` and `V8_MINOR_VERSION`.

---

## V8 API changes

**V8 API diff between two versions:**
`https://github.com/v8/v8/compare/X.Y-lkgr...A.B-lkgr`
Replace `X.Y` and `A.B` with the old and new V8 version numbers (e.g., `12.9-lkgr...13.0-lkgr`).
Focus on changes to files under `include/` — those are the public API.

**V8 deprecation notices** (searching for a specific function):
Web search: `site:github.com/v8/v8 "IsExternal" deprecated`
Or browse: `https://github.com/v8/v8/commits/main/include/v8-string.h`

**V8 blog** (major API changes are announced here):
`https://v8.dev/blog`

---

## nan and node-addon-api (header path providers)

These packages are not researched for API compatibility — they only need to resolve
Node.js headers correctly for the new version. For upgrade instructions (including
how to pin versions correctly), see Step 3 of SKILL.md.

For reference only (not required for a standard compat check):
- NAN releases: `https://github.com/nodejs/nan/releases`
- node-addon-api CHANGELOG: `https://github.com/nodejs/node-addon-api/blob/main/CHANGELOG.md`

---

## Quick search templates

| What to find | How to find it |
|---|---|
| V8 version in Node X | Web search: `Node.js X.0.0 release V8 version` |
| Specific V8 API change | Web search: `V8 "<api_name>" removed OR deprecated site:github.com/v8/v8` |
| V8 String API commit history | Browse: `https://github.com/v8/v8/commits/main/include/v8-string.h` |
