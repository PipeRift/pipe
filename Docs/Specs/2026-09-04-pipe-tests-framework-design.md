# PipeTests Framework — Design

Date: 2026-09-04
Status: Draft (pending spec review)

## Problem

The Pipe and Rift test suites rely on the vendored **Bandit** third-party test framework (`Extern/Pipe/Extern/Bandit/`) plus its bundled **Snowhouse** assertion library. Bandit is heavy, third-party, and its capture-scoping API does not fit the project's taste. We want to replace it step by step with a small, Pipe-native test framework owned by the project, following Pipe code style and the project's "simple solutions, C-like cpp, minimal templates, comments only when code is not enough" preference.

## Goals

- Provide a native test framework module **`PipeTests`** in the Pipe source tree.
- Used by **both** `PipeTests` and `RiftTests`.
- Mirror the bandit structure (`Spec`/`Describe`/`It`/`XIt`/`BeforeEach`/`AfterEach`) so migration is mechanical.
- Replace Snowhouse `AssertThat(x, Equals(y))` with a fluent `Expect(x).ToEqual(y)` style.
- Detailed failure messages (actual vs expected, `file:line`), extensible for user types.
- Remove the Bandit dependency entirely **only at the very end**, once the framework is proven.
- No new runtime burden on the shipped `Pipe` library.

## Non-Goals

- No exception-based matchers (`ToThrow`). Project disables RTTI project-wide and no current test throws; YAGNI.
- No (or minimal) CLI reporter/arg machinery in the first iteration.
- Not migrating the existing bandit tests or removing Bandit until the framework is complete and self-tested (see Migration Phasing).

## API Design

Style: **imgui-style global context**. All functions are global in namespace `p`; the user writes `using namespace p;`. There is no capture-scoped "only relevant functions" context — context is global and tracked as functions are called. The exception is `Spec`, which is self-registering.

```cpp
using namespace p;

// Spec(name, fn) = go_bandit + first-level named group
Spec("Group", []() {
    BeforeEach([]() { /* setup */ });
    AfterEach([]() { /* teardown */ });

    Describe("Sub", []() {
        It("does something", []() {
            Expect(value).ToEqual(4);
            Expect(value).ToNotEqual(5);
            Expect(value).ToBeLess(5);
            Expect(value).ToBeLessOrEqual(4);
            Expect(value).ToBeGreater(2);
        });
        XIt("disabled test", []() { /* never runs */ });
    });

    It("top-level test", []() {
        Expect("acidic").ToContain("acid");
        Expect(flag).ToBeTrue();
        Expect(other).ToBeFalse();
    });
});
```

### Top-level semantics

- `Spec(name, fn)` — self-registering top-level entry that **also** opens a first-level group named `name`. This is the go_bandit + first `Describe` replacement.
- `Spec(fn)` (nameless) — registers the top level only (like `go_bandit`), adding no extra group; `Describe` is then used inside it.
- `Describe` is **runtime-checked** to only be valid inside a `Spec`. If used outside a `Spec`: **log an error and ignore** the offending group (no exception).

### Functions (namespace `p`)

| Function | Role |
|----------|------|
| `Spec(name, fn)` / `Spec(fn)` | Self-registering top-level; optional first named group |
| `Describe(name, fn)` | Push a nested group; runtime-checked inside a `Spec` |
| `It(name, fn)` | Register a runnable leaf test in the current group |
| `XIt(name, fn)` | Register a leaf test marked **skipped** (never runs) |
| `BeforeEach(fn)` | Setup hook attached to the current group |
| `AfterEach(fn)` | Teardown hook attached to the current group |
| `Expect(value)` | Returns a fluent matcher over `value` |
| `RunTests(argc, argv)` | Runs the suite; returns process exit code |

### Assertions — `Expect(value)`

Fluent matcher methods, naming in Pipe CamelCase:

- `ToEqual(x)` / `ToNotEqual(x)`
- `ToBeLess(x)` / `ToBeLessOrEqual(x)` / `ToBeGreater(x)` / `ToBeGreaterOrEqual(x)`
- `ToBeTrue()` / `ToBeFalse()`
- `ToContain(sub)` / `ToNotContain(sub)` (strings / containers)

### Failure reporting

- On failure: print `file:line`, a description, and the **actual vs expected** values (detailed).
- Value stringification uses an **extensible formatter** customization point.
  - Default support: arithmetic types via `std::format`/`std::to_string`; `StringView` / `const char*` / `std::string_view` for strings.
  - Users extend by specializing/overloading a `TestFormatter<T>` (or `TestFormat(value)`) hook for their own types.
  - Prefer `StringView` over `String`.

## Implementation

### New files (in the Pipe submodule)

- `Extern/Pipe/Include/PipeTests.h` — public API (global functions, `Expect` matcher, formatter hook). Mostly templates/macros.
- `Extern/Pipe/Src/PipeTests.cpp` — global registration cursor (current-group stack), test registry, `p::RunTests(int, char**)`.

### Build — separate target, not into the runtime Pipe lib

`Extern/Pipe/CMakeLists.txt`:

- Define `add_library(PipeTests ...)` **unconditionally** (alongside `Pipe`, **before** the `PIPE_BUILD_TESTS` gate) so Rift can consume it via the submodule.
- **Exclude `Src/PipeTests.cpp`** from Pipe's `file(GLOB_RECURSE ... Src/*.cpp)` (currently line 65) so the test framework does **not** ship in the runtime `Pipe` library. Add it only to the `PipeTests` target.
- Give `PipeTests` the standard Pipe setup (`pipe_target_define_platform`, `pipe_target_enable_CPP20`, `pipe_target_disable_rtti`, `pipe_target_shared_output_directory`) and link `Pipe`; expose `Include/`.

### Runner

`p::RunTests(int argc, char* argv[]) -> int`:

- Iterates the registered test tree.
- Runs each test, skipping `XIt`.
- Reports pass/fail/skip counts plus the names/locations of failures.
- Returns a process exit code (`0` when all pass).
- Minimal `argc`/`argv` handling now; signature kept for future `--filter` support.

### Global registration cursor

A current-group stack in `PipeTests.cpp`. `Describe` pushes its group, runs `fn` (children register against it via the global functions), then pops. `It`/`XIt`/`BeforeEach`/`AfterEach` attach to the current group.

## Migration Phasing

**Deliberate: do NOT migrate existing tests and do NOT remove third-party libraries until the very end, when the framework is done and self-tested.** Bandit stays linked and coexists throughout development.

1. **Add `PipeTests` module** — header + source; `add_library(PipeTests)`; source-glob exclusion. Build succeeds.
2. **Self-test the framework with small new tests** (no migration of existing tests):
   - Create small **new** framework tests in `Extern/Pipe/Tests/PipeTests/` (e.g. `PipeTests.spec.cpp`) written with the new API to validate: `Spec`/`Describe`/`It`/`XIt`/`BeforeEach`/`AfterEach`, all `Expect` matchers, failure reporting, skip counting, `RunTests` exit code, and `Describe`-outside-`Spec` behavior.
   - Wire a **separate small runner** (its own `main.cpp` calling `p::RunTests`) for this smoke target, running **alongside** the existing bandit `PipeTests` executable.
   - Verify via `ctest` that **both** the new self-tests and the untouched bandit suite pass. Iterate until the framework is proven.
3. **Final flip (LAST, only when the system is done):**
   - Migrate existing `*spec.cpp` files file-by-file (transform map below).
   - `Extern/Pipe/Tests/CMakeLists.txt`: link `Pipe` + `PipeTests`, drop `Bandit`; `main.cpp` → `p::RunTests(argc, argv)`; drop `--reporter=spec`.
   - Migrate Rift `Tests/*.spec.cpp` + `Tests/CMakeLists.txt`: replace `Bandit` with `PipeTests`.
   - Remove `Bandit`: delete the `Bandit` INTERFACE target from `Extern/Pipe/Extern/CMakeLists.txt`, remove the vendored `Extern/Pipe/Extern/Bandit/` directory, strip remaining bandit includes.
   - Final full `ctest` + `ClangFormat`/`ClangTidy` pass.

### Transform map (for step 3)

| Before (bandit) | After (PipeTests) |
|-----------------|-------------------|
| `#include <bandit/bandit.h>` + `using namespace snowhouse; using namespace bandit;` | `#include <PipeTests.h>` + `using namespace p;` |
| `go_bandit([](){ describe("G", ...) })` | `Spec("G", [](){ ... })` |
| `describe(...)` | `Describe(...)` |
| `it(...)` | `It(...)` |
| `xit(...)` | `XIt(...)` |
| `before_each(...)` | `BeforeEach(...)` |
| `after_each(...)` | `AfterEach(...)` |
| `AssertThat(v, Equals(x))` | `Expect(v).ToEqual(x)` |
| `AssertThat(v, !Equals(x))` | `Expect(v).ToNotEqual(x)` |
| `AssertThat(x, Equals(true/false))` | `Expect(x).ToBeTrue()/ToBeFalse()` |
| `<` / `<=` relations | `ToBeLess` / `ToBeLessOrEqual` |
| contains checks | `ToContain` / `ToNotContain` |

Note: existing test files that `using namespace snowhouse; using namespace bandit;` also add `using namespace p;` where Pipe types are used (e.g. `StringView.spec.cpp`). After migration these files use only `using namespace p;`.

## Wiring facts (verified)

- `Bandit` INTERFACE target is defined in `Extern/Pipe/Extern/CMakeLists.txt`, added **unconditionally** (Pipe `CMakeLists.txt` line 47 `add_subdirectory(Extern)` runs before the `PIPE_BUILD_TESTS` gate).
- Pipe supplies its own tests executable gated behind `PIPE_BUILD_TESTS` (default `PIPE_IS_PROJECT`, i.e. ON when Pipe is the top project).
- Rift builds its **own** `RiftTests` executable in `Tests/CMakeLists.txt`, linking `RiftASTLib` + `Bandit` (imported through Pipe's `Extern`).
- Therefore the `PipeTests` target must be defined **unconditionally** in Pipe's `CMakeLists.txt` so Rift can link it.

## Constraints & style

- Namespace `p`; CamelCase functions; `camelBack` parameters/variables; tabs; 100-col limit; `.clang-format` (Microsoft base).
- Minimal templates; comments only where code is insufficient.
- No exceptions / no RTTI (`-fno-rtti` project-wide).
- Prefer `StringView` over `String`.

## Open Questions

- Confirmed during design: no `ToThrow` (see Non-Goals); `Describe` misuse logs + ignores; `RunTests(int, char**)` signature; API names `Spec/Describe/It/XIt/BeforeEach/AfterEach/Expect` in Pipe CamelCase.

## Out of Scope / Follow-ups

- CLI `--filter` / reporter selection (deferred; `RunTests` keeps `argc`/`argv` for future use).
