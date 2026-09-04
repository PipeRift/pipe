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

Style: **imgui-style global context**. All functions are global in namespace `p`; the user writes `using namespace p;`. There is no capture-scoped "only relevant functions" context — context is global and tracked as functions are called.

**Macro-free registration (2026-09-04):** The framework uses **no macros** — `Spec`/`Describe`/`It`/`XIt`/`BeforeEach`/`AfterEach` are plain functions. Because a bare function call is ill-formed at namespace scope (C++ permits only declarations there), specs **cannot self-register at global scope**. Registrations run from a function: each spec file exports a `RegisterXxxTests()` routine, and the test executable's `main()` calls each one before `p::RunTests`. `Spec(name, fn)` opens a first-level named group; `Spec(fn)` registers the top level only.

```cpp
using namespace p;

// Registration runs inside a function — never at namespace scope (no macros).
void RegisterGroupTests()
{
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
}
```

### Top-level semantics

- `Spec(name, fn)` — top-level entry that **also** opens a first-level group named `name`. This is the go_bandit + first `Describe` replacement.
- `Spec(fn)` (nameless) — registers the top level only (like `go_bandit`), adding no extra group; `Describe` is then used inside it.
- `Describe` is **runtime-checked** to only be valid inside a `Spec`. If used outside a `Spec`: **log an error and ignore** the offending group (no exception).

### Functions (namespace `p`)

| Function | Role |
|----------|------|
| `Spec(name, fn)` / `Spec(fn)` | Top-level group; called from a `Register*Tests()` routine |
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

- `Extern/Pipe/Include/PipeTests.h` — public API (global functions, `Expect` matcher, formatter hook). Mostly templates; **no macros**.
- `Extern/Pipe/Src/Tests/PipeTests.cpp` — function-local static `TestContext`, registration functions (`Spec`/`Describe`/`It`/`XIt`/`BeforeEach`/`AfterEach`), `p::RunTests(settings)` + `p::RunTests(int, char**)` argv forwarder.

### Build — separate target, not into the runtime Pipe lib

`Extern/Pipe/CMakeLists.txt`:

- Define `add_library(PipeTests ...)` (alias `Pipe::Tests`) **unconditionally** (alongside `Pipe`, **before** the `PIPE_BUILD_TESTS` gate) so Rift can consume it via the submodule. (The suite executable is `PipeTesting`, alias `Pipe::Testing`, so the `PipeTests` name is free for the framework library.)
- **Exclude `Src/Tests/PipeTests.cpp`** from Pipe's `file(GLOB_RECURSE ... Src/*.cpp)` (currently line 65) so the test framework does **not** ship in the runtime `Pipe` library. Add it only to the `PipeTests` target.
- Give `PipeTests` the standard Pipe setup (`pipe_target_define_platform`, `pipe_target_enable_CPP20`, `pipe_target_disable_rtti`, `pipe_target_shared_output_directory`) and link `Pipe`; expose `Include/`.

### Runner

`p::RunTests(const TestSettings&) -> int` (plus an `argc`/`argv` overload that parses `--filter=X` / `--filter X` / positional into settings and forwards, so other systems can run tests without text args):

- Iterates the registered test tree, running only tests whose full name contains `settings.filter` (empty filter runs all).
- Runs each test, skipping `XIt`.
- Reports pass/fail/skip counts plus the names/locations of failures.
- Returns a process exit code (`0` when all pass).

### Global registration cursor

A current-describe cursor in `PipeTests.cpp` (`TestContext::currentDescribe`, accessed via `GetTestContext()`/`CurrentDescribe()`). `Describe` pushes its describe, runs `fn` (children register against it via the global functions), then pops. `It`/`XIt`/`BeforeEach`/`AfterEach` attach to the current describe. Pipe types throughout: `i32` counters, `TFunction<void()>` for immediately-invoked `Spec`/`Describe` callbacks, owning `std::function` for stored test bodies/hooks (`TFunction` is non-owning and would dangle).

## Migration Phasing

**Deliberate: do NOT migrate existing tests and do NOT remove third-party libraries until the very end, when the framework is done and self-tested.** Bandit stays linked and coexists throughout development.

1. **Add `PipeTests` module** — header + source; `add_library(PipeTests)`; source-glob exclusion. Build succeeds.
2. **Self-test the framework with small new tests** (no migration of existing tests):
   - Create small **new** framework tests in `Extern/Pipe/Tests/PipeTests/` (e.g. `PipeTests.spec.cpp`) written with the new API to validate: `Spec`/`Describe`/`It`/`XIt`/`BeforeEach`/`AfterEach`, all `Expect` matchers, failure reporting, skip counting, `RunTests` exit code, and `Describe`-outside-`Spec` behavior. The spec file holds `Spec(...)` at file scope (auto-registers).
   - Wire a **separate small runner** (its own `main.cpp` calling `p::RunTests`) for this smoke target, running **alongside** the existing bandit `PipeTesting` executable.
   - Verify via `ctest` that **both** the new self-tests and the untouched bandit suite pass. Iterate until the framework is proven.
3. **Final flip (LAST, only when the system is done):**
   - Migrate existing `*spec.cpp` files file-by-file (transform map below). Each migrated file holds `Spec(...)` at file scope (auto-registers; no wrapper, no `main` changes).
   - `Extern/Pipe/Tests/CMakeLists.txt`: rename suite exe to `PipeTesting`, link `Pipe` + `PipeTests`, drop `Bandit`; `main.cpp` calls `p::RunTests(argc, argv)`; drop `--reporter=spec`.
   - Migrate Rift `Tests/*.spec.cpp` + `Tests/CMakeLists.txt`: replace `Bandit` with `Pipe::Tests` (the alias); Rift's `Tests/main.cpp` only swaps `bandit::run` for `p::RunTests`.
   - Remove `Bandit`: delete the `Bandit` INTERFACE target from `Extern/Pipe/Extern/CMakeLists.txt`, remove the vendored `Extern/Pipe/Extern/Bandit/` directory, strip remaining bandit includes.
   - Final full `ctest` + `ClangFormat`/`ClangTidy` pass.

### Transform map (for step 3)

| Before (bandit) | After (PipeTests) |
|-----------------|-------------------|
| `#include <bandit/bandit.h>` + `using namespace snowhouse; using namespace bandit;` | `#include <PipeTests.h>` + `using namespace p;` |
| `go_bandit([](){ describe("G", ...) })` (global scope) | File-scope `Spec("G", [](){ ... })`; auto-registers, no `main` changes |
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
- Therefore the `PipeTests` target (alias `Pipe::Tests`) must be defined **unconditionally** in Pipe's `CMakeLists.txt` so Rift can link it.

## Constraints & style

- Namespace `p`; CamelCase functions; `camelBack` parameters/variables; tabs; 100-col limit; `.clang-format` (Microsoft base).
- Minimal templates; comments only where code is insufficient.
- No exceptions / no RTTI (`-fno-rtti` project-wide).
- Prefer `StringView` over `String`.

## Open Questions

- Confirmed during design: no `ToThrow` (see Non-Goals); `Describe` misuse logs + ignores; `RunTests(settings)` + `RunTests(int, char**)` signatures; API names `Spec/Describe/It/XIt/BeforeEach/AfterEach/Expect` in Pipe CamelCase.

## Out of Scope / Follow-ups

- Reporter selection (deferred).
