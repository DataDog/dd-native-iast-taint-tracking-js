# AGENTS.md

## Project Overview

`@datadog/native-iast-taint-tracking` is a Node.js native C++ addon that implements taint tracking
for Datadog's IAST (Interactive Application Security Testing) solution. It tracks how untrusted
("tainted") data flows through string operations at the C++ layer, enabling detection of injection
vulnerabilities without instrumentation overhead in JavaScript.

The addon exposes a transaction-based API: callers open a transaction, mark strings/objects as
tainted, perform string operations (concat, slice, replace, etc.), and query whether results are
still tainted and from which source.

When the native build is unavailable, `index.js` automatically falls back to a no-op
implementation that passes values through unchanged; taint tracking is silently disabled.

`replace.js` is the exception: unlike all other operations which are implemented in C++,
`replace()` lives in JavaScript to handle regex edge cases that cannot be expressed cleanly
at the C++ layer.

---

## Prerequisites

- **Node.js >= 18** (supported up to 26)
- **npm** (use npm for scripts; no yarn needed in this repo)
- **node-gyp** (and its [system dependencies](https://github.com/nodejs/node-gyp#installation):
  Python, make, C++ compiler)
- **CppUTest + CMake** — both required for C++ unit tests (`npm run test:native`):
  ```sh
  # Debian/Ubuntu
  apt install libcpputest-dev cpputest cmake
  ```
- **Valgrind** or **AddressSanitizer** optional, for memory-safety test runs

Install JavaScript dependencies:
```sh
npm install
```

---

## Directory Structure

```
.
├── src/                        # C++ implementation
│   ├── api/                    # V8 bindings for each string operation
│   │   ├── taint_methods.cc    # Core API: createTransaction, newTaintedString, etc.
│   │   ├── concat.cc / trim.cc / slice.cc / replace.cc / ...
│   │   └── metrics.cc
│   ├── tainted/                # Core taint model
│   │   ├── transaction.h/cc    # Transaction lifecycle and tainted-object registry
│   │   ├── tainted_object.h/cc # Represents a tainted value
│   │   ├── range.h/cc          # [start, end) taint ranges with InputInfo
│   │   └── input_info.h/cc     # Source metadata (parameter name, type)
│   ├── container/              # Memory primitives
│   │   ├── pool.h / queued_pool.h  # Object pools (avoid per-call allocation)
│   │   ├── weakmap.h           # WeakMap: pointer → tainted object
│   │   └── shared_vector.h     # Thread-safe vector for ranges
│   ├── gc/                     # V8 GC integration
│   ├── utils/                  # String helpers, V8 utilities, propagation logic
│   ├── iast.cc / iast.h        # Global IAST state
│   ├── iastlimits.h            # Hard limits (MAX_TAINTED_OBJECTS=4096, MAX_RANGES=50)
│   └── transaction_manager.h   # Transaction lifecycle management
├── test/
│   ├── js/                     # Mocha JavaScript tests (one spec per API method)
│   └── cpputest/               # CppUTest C++ unit tests
├── scripts/
│   ├── cpputest.sh             # Build + run C++ tests via CMake
│   └── libc.js                 # libc detection for prebuilt selection
├── docker/                     # Dockerfiles for multi-libc test environments
├── suppressions/
│   └── node.supp               # Valgrind suppressions for Node.js false-positives
├── index.js                    # Main entry: loads native addon, no-op fallback
├── replace.js                  # replace() lives in JS, not C++ — handles regex edge cases
├── index.d.ts                  # TypeScript types
├── binding.gyp                 # node-gyp build config
└── CMakeLists.txt              # CMake config for C++ tests
```

---

## Build

```sh
# Standard build (produces build/Release/iastnativemethods.node + dist/ copy)
npm run build

# ARM64
npm run build:arm

# Debug builds (disable optimizations, add instrumentation)
npm run build:asan       # AddressSanitizer
npm run build:valgrind   # Debug symbols for Valgrind
```

The built artifact is a `.node` file loaded at runtime by `node-gyp-build`. Prebuilts for
`linux-x64`, `darwin-x64`, and `win32-x64` are shipped in `prebuilds/` and are generated
automatically by CI on release (`.github/workflows/release.yml`) — do not commit them manually.

If the build fails, verify all [Prerequisites](#prerequisites) are met first.

**Compiler flags (binding.gyp):** C++20, `-Os` (size optimization), `-fvisibility=hidden`,
`-flto`, dead-section stripping. These are intentional, do not remove them without measuring
the size/performance impact.

---

## Testing

### When to Run What

| Change | Minimum test suite |
|---|---|
| JavaScript (`index.js`, `replace.js`) | `npm test` |
| C++ source (`src/`) | `npm test` + `npm run test:native` |
| Memory-sensitive C++ (pools, limits, GC) | `npm test` + `npm run test:native` + `npm run test:js-asan` |
| libc detection / prebuilt loading | `npm run test:docker` |

### JavaScript Tests (Mocha)

Requires a prior `npm run build` if no prebuilt is available for your platform.

```sh
npm test                 # run all specs under test/js/
npm run test:js-junit    # same, JUnit XML output (CI)
```

Tests are in `test/js/`, one `*.spec.js` file per API method. The `test/mocha.opts` sets a
4000 ms timeout. Use `--grep` to narrow:

```sh
./node_modules/.bin/mocha --recursive --grep "concat"
```

### C++ Unit Tests (CppUTest)

```sh
npm run test:native      # build via CMake and run
npm run test:junit       # same, JUnit XML output (CI)
```

The script is `scripts/cpputest.sh`. It runs `cmake .. && make` inside
`test/cpputest/build/`, then executes the `native_test` binary.

### Memory-Safety Tests

```sh
# AddressSanitizer (builds with -fsanitize=address, then runs)
npm run test:asan        # C++ tests with ASAN
npm run test:js-asan     # JS tests with ASAN preloaded

# Valgrind
npm run test:js-valgrind
```

Valgrind suppressions for known Node.js false-positives are in `suppressions/node.supp`.

### Docker Tests (multi-libc)

```sh
npm run test:docker
```

Runs the full JS test suite inside Debian (glibc) and Alpine (musl) containers across multiple
variants. Mirrors the CI `cpp-test-libc` and `cpp-test-musl` jobs.

### Test Philosophy

- JavaScript tests are **integration tests**: they exercise the real native addon end-to-end.
- C++ tests are **unit tests**: they exercise transaction management and container internals
  without the V8 layer.
- A bug fix must include a test case that reproduces the failure. Memory-unsafety bugs must
  also be verified with the ASAN or Valgrind target.

---

## Code Style and Linting

### JavaScript

Lint with:
```sh
npm run lint
```

Config: `.eslintrc.json` (extends `eslint:recommended` + `standard`).

Key rules:
- `ecmaVersion: 2020`
- `max-len: 120` characters
- `no-var` — use `const`/`let`
- `prefer-const`
- `object-curly-spacing: always` — `{ key: value }`
- `no-console` — no console output in production code
- `import/no-extraneous-dependencies` — only listed devDependencies

### C++

`cpplint` must be installed separately (it is not an npm script). It runs as a dedicated job in
CI; to run it locally:

```sh
cpplint --filter=-build/include_subdir,-whitespace/indent_namespace \
        --linelength=120 --recursive ./src
```

Follows Google C++ style. Key conventions:
- C++20 standard (`-std=gnu++20`)
- All new code goes in the `iast` namespace (or `iast::api`, `iast::tainted`, etc.)
- Header guards use the full path: `#ifndef SRC_TAINTED_RANGE_H_`
- Use `auto` for local variables when the type is obvious from the right-hand side
- Prefer range-based `for` over index loops where applicable

---

## Performance and Memory Considerations

This addon executes in the hot path of every instrumented string operation. Treat every
function called per-request as latency-sensitive.

**Object pooling is the primary memory strategy.** `container::Pool` and `container::QueuedPool`
pre-allocate `Range`, `InputInfo`, and `TaintedObject` instances. Avoid `new` / `delete` inside
request-scoped code; always allocate from the transaction's pool.

**Hard limits** (defined in `src/iastlimits.h`):
- `MAX_TAINTED_OBJECTS = 4096` per transaction
- `MAX_RANGES = 50` per tainted object
- `MAX_GLOBAL_TAINTED_RANGES = 204800`

These are intentionally conservative to bound memory usage per request. To change them, edit
`src/iastlimits.h`, run `npm run build`, and verify with `npm run test:js-asan` that no
allocator overflows occur under the new limits.

**V8 string internals:** The module uses `ExternalString` resources (via NAN) to attach
taint metadata to V8 strings without copying. Short strings (length < 10) are handled
specially in `NewTaintedString` to avoid V8 string cache collisions. Do not remove that
branch without understanding V8's string interning.

**GC integration:** `src/gc/gc.cc` hooks V8's garbage collector to clean up tainted objects
when their corresponding V8 strings are collected. Any new taint registration must also
register the GC callback.

---

## Development Workflow

### Adding a New Taint Operation

1. Add a C++ source file under `src/api/<operation>.cc` following the pattern in
   `src/api/concat.cc`.
2. Register the new function in `src/api/taint_methods.cc` (`TaintMethods::Init`).
3. Add the source path to `binding.gyp` → `sources`.
4. Export from `index.js` and add a no-op fallback in the catch block.
5. Add TypeScript types to `index.d.ts`.
6. Add a `test/js/<operation>.spec.js` spec file covering: normal taint propagation, untainted
   input passthrough, and invalid argument handling.

---

## Pull Requests and CI

### PR Requirements

- Use the template from `.github/pull_request_template.md`
- Label: `semver-patch` (fixes only), `semver-minor` (new features), `semver-major` (breaking)
- All tests (JavaScript and C++) must be updated and pass — all-green policy, no exceptions

### CI

CI runs on GitHub Actions (`.github/workflows/`). On every PR it runs: JS lint, C++ lint, JS
tests across Node 18–26, C++ unit tests, Valgrind, AddressSanitizer, and Docker tests on both
glibc (Debian) and musl (Alpine) variants.

---

## Error Handling

**The addon must never throw uncaught exceptions or crash the host process.** All V8 binding
functions follow this pattern:

1. Validate argument count and types; throw a `TypeError` back to JS if invalid.
2. Set the return value early to a safe default (usually the input value unchanged).
3. Wrap allocation-heavy logic in `try/catch` for `std::bad_alloc` and pool exceptions.
4. On allocation failure, silently return the safe default; taint tracking degrades
   gracefully, the application continues.

Do not add `throw` statements outside of these exception types. Propagate errors upward
only when the caller is already inside a try/catch block at the API boundary.

### Debugging Crashes

For segfaults during JS tests, the `segfault-handler` devDependency prints a stack trace
before the process exits. Enable it in a test by requiring it at the top:
```js
require('segfault-handler').registerHandler('crash.log')
```

Use `npm run test:js-valgrind` for memory error traces (heap corruption, use-after-free).
Use `npm run test:js-asan` for faster in-process detection.
