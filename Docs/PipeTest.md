---
title: Pipe Test
---
**Header:** [`<PipeTest.h>`](../Include/PipeTest.h)
**Namespace:** `p`
**Uses:** [`PipeReflect`](./PipeECS.md) [`PipeStrings`](./PipeStrings.md) [`PipeTime`](./PipeTime.md)

## Overview

`PipeTest` is test framework built into Pipe, used by the library to test itself.
It is heavily inspired by `Bandit`, following similar APIs and outputs to support existing IDE tools built for that library.

```cpp
#include <PipeTest.h>

using namespace p;

P_SPEC("Math", []()
{
    BeforeEach([]() { /* runs before each test */ });

    It("clamps to range", []()
    {
        Expect(Clamp(5, 0, 3)).ToEqual(3);
    });

    XIt("disabled case", []()
    {
        // never runs
    });

    AfterEach([]() { /* runs after each test */ });
});
```

## Philosophy

A few guiding decisions shape the whole test API.

- **Macro-free registration:** Registration of tests (`Describe`, `It`, `XIt`, `BeforeEach`, `AfterEach`) are plain functions. The only macro is `P_SPEC`, which it is just syntax sugar to statically register an spec.
- **Immediate global context:** All registration functions live in namespace `p` and operate on a global `TestContext`.
- **Fluent assertions:** `Expect(x)` followed by `ToEqual(y)`, `ToBeTrue()`, `ToContain(sub)`, etc to cover many different testing scenarios.
- **Extensible value formatting:** Failure messages are messaged in a human-readable way, and extensible by the user for new types.
- **Mirror bandit's vocabulary:** `Describe`/`It`/`XIt`/`BeforeEach`/`AfterEach`, `--only`/`--skip`/`--reporter`/`--dry-run`, `Spec`/`Dots`/`Singleline`/`Info` reporters. Migrating existing tests is trivial.
- **No exceptions / no RTTI:** Pipe doesn't require or use RTTI. This is the same with PipeTests.

## Writing Tests
On any cpp file in our test module, we can define one or more specs.

```cpp
// Specs register tests inside and optionally adds root Describe("Files.Paths")
P_SPEC("Files.Paths", []() 
{
    BeforeEach([]() { /* setup */ });

    Describe("Join", []()
    {
	    // Describes group all tests inside. They can be nested.
        It("joins two segments", []()
        {
            Expect(Join("a", "b")).ToEqual("a/b");
        });
        
        It("joins three segments", []()
        {
            Expect(Join("a", "b", "c")).ToEqual("a/b/c");
        });

        XIt("join four segments", []() { /* 'XIt' are disabled tests */ });
    });

    It("top-level test", []()
    {
        Expect(true).ToBeTrue();
        Expect(false).ToBeFalse();
    });

    AfterEach ([]() { /* teardown */ });
});
```

> [!Note]
> Because registration runs from a constructor (`SpecAutoRegister`), `P_SPEC` can only appear at namespace scope. Inside a function, register the spec directly with `RegisterSpec(name, callback)`.

## Running Tests

`PipeTest` exposes two runner entry points:

```cpp
int RunTests(const TestSettings& settings);
int RunTests(int argc, char** argv);
```

The second overload parses CLI args into `TestSettings`.

In CLI, simply call your test binary. You can use these args:

| Argument                              | Effect                                                          |
| ------------------------------------- | --------------------------------------------------------------- |
| `--only=<substring>`                  | Run only describes/its whose full name contains `<substring>`   |
| `--skip=<substring>`                  | Skip describes/its whose full name contains `<substring>`       |
| `--break-on-failure`                  | Stop on the first failing test                                  |
| `--dry-run`, `-l`, `--list`           | Report the full tree as SKIPPED, run nothing                    |
| `--report-timing`                     | Report per-test and total run durations                         |
| `-r=<name>`, `--reporter=<name>`      | Select reporter by (case-insensitive) name — see [Reporter Name Lookup](#Reporter-Name-Lookup) |
| `--colorizer=off`, `--no-color`, `-c` | Disable colorized output                                        |
| `--version`                           | Print Pipe version and exit                                     |
| `--help`                              | Print usage and exit                                            |

## Reporters

Reporters share the same per-test execution flow but format the run their own way:

| Reporter            | Style                                                                                                                                                                             |
| ------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `spec`<br>(default) | Indented contexts, `- it <name> ... OK` lines, summary block. Compatible with the Bandit VSCode adapter (one tab per indent level — any other indent breaks hierarchy detection). |
| `dots`              | One character per test, color-coded, wrapped at a fixed line width.                                                                                                               |
| `singleline`        | Live overwriting status line on real terminals (`Executed N tests.`). On redirected streams it prints nothing per-test, only the final summary.                                   |
| `info`              | Verbose: `begin/end <context>` lines, per-context totals (`{n} total`, `{n} skipped`, `{n} failed`), `[ PASS ]`/`[ FAIL ]`/`-ERROR->` test lines, and a final failures list.      |

`--report-timing` adds a colored duration suffix (e.g. ` (0.00012s)` / `(0.4us)`) to every test line and a total-time line at the end in the `info` reporter.

Custom reporters are supported by inheriting `p::ITestReporter`.
They can be called by their name (`--reporter=SpecialReporter` or just `--reporter=Special`):
```cpp
struct SpecialReporter : p::ITestReporter
{
    using Super = p::ITestReporter;
    P_STRUCT(SpecialReporter)

    // ... override the reporter callbacks ...
};
```

## Filters & Behavior

- Tests not matched by `--only`, matched by `--skip`, marked `XIt`, or after a `--break-on-failure` trigger are all reported as **SKIPPED** (bandit semantics). They are not hidden — only excluded from execution.
- Failures are buffered during the run and emitted by the reporter at the end, so they don't interleave with deferred context output.

## IDE Tools
### VSCode
You can use extension [Bandit Test Explorer](https://marketplace.visualstudio.com/items?itemName=dampsoft.vscode-banditcpp-test-adapter):
![[BanditTestExplorer.png]]

---

## Reference
### `P_SPEC` Macro
Automatically registers an battery of tests.

```cpp
P_SPEC("Topic", []()
{
    // body
});
```

```cpp
#define P_SPEC static const p::SpecAutoRegister P_CAT(_pipeSpecReg_, __COUNTER__)
```
### Registration Functions

All in namespace `p`. Bodies in `P_SPEC` / `Describe` are stored as `TFunction<void()>` (non-owning, called immediately during registration). Test bodies and hooks are stored as `std::function<void()>` (owning) because they run later from `RunTests`.

| Function           | Signature                                                      | Purpose                                                                            |
| ------------------ | -------------------------------------------------------------- | ---------------------------------------------------------------------------------- |
| `RegisterSpec`     | `(StringView name, TFunction<void()> fn)`                      | Internal: pushes a named first-level describe, runs `fn`, pops.                    |
| `RegisterSpec`     | `(TFunction<void()> fn)`                                       | Runs `fn` against the virtual root describe.                                       |
| `SpecAutoRegister` | ctor `(StringView, TFunction<void()>)` / `(TFunction<void()>)` | Static-init registrar used by `P_SPEC`.                                            |
| `Describe`         | `(StringView name, TFunction<void()> fn)`                      | Push a nested describe. Runtime-checked inside a `Spec`; logs + ignores otherwise. |
| `It`               | `(StringView name, std::function<void()> fn)`                  | Register a runnable leaf test.                                                     |
| `XIt`              | `(StringView name, std::function<void()> fn)`                  | Register a leaf test that is never run.                                            |
| `BeforeEach`       | `(std::function<void()> fn)`                                   | Attach a setup hook to the current describe.                                       |
| `AfterEach`        | `(std::function<void()> fn)`                                   | Attach a teardown hook to the current describe.                                    |

### `Expect(value)` — Fluent Matchers

`Expect(value, loc = std::source_location::current())` returns an `ExpectValue<T>` bound to the caller's `file:line`. The matchers available today:

| Matcher | Signature | Failure message |
|---------|-----------|-----------------|
| `ToEqual(expected)` | `template<typename E> void ToEqual(const E&) const` | `Expected {actual} to equal {expected}` |
| `ToNotEqual(expected)` | `template<typename E> void ToNotEqual(const E&) const` | `Expected {actual} to not equal {expected}` |
| `ToBeLess(other)` | `void ToBeLess(const Actual&) const` | `Expected {actual} to be less than {other}` |
| `ToBeLessOrEqual(other)` | `void ToBeLessOrEqual(const Actual&) const` | `Expected {actual} to be less or equal to {other}` |
| `ToBeGreater(other)` | `void ToBeGreater(const Actual&) const` | `Expected {actual} to be greater than {other}` |
| `ToBeGreaterOrEqual(other)` | `void ToBeGreaterOrEqual(const Actual&) const` | `Expected {actual} to be greater or equal to {other}` |
| `ToBeTrue()` | `void ToBeTrue() const` | `Expected value to be true` |
| `ToBeFalse()` | `void ToBeFalse() const` | `Expected value to be false` |
| `ToContain(sub)` | `void ToContain(StringView) const` | `Expected {actual} to contain {sub}` |
| `ToNotContain(sub)` | `void ToNotContain(StringView) const` | `Expected {actual} to not contain {sub}` |

- **String-aware equality.** `ToEqual`/`ToNotEqual` treat string-like operands (`StringView`, `const char*`, `std::string`, `p::String`) as strings and compare via `StringView`; everything else uses `operator==`.
- **Containment.** `ToContain`/`ToNotContain` require a `StringView` substring; the actual value is constructed into a `StringView` view for the search.
- **Source location.** The matcher's `file:line` is captured at the call site via `std::source_location` defaults, so failure reports point at the assertion line, not inside the framework.
- **Assertion counter.** Every matcher increments an internal counter. Tests that ran zero asserts are surfaced by reporters as "no assertions" rather than `[ PASS ]`.

## `TestString<T>` — Custom Value Formatting

Failure messages format actual/expected values through `String TestString<T>(const T&)`. The default implementation handles two cases:
- **Formattable types** (`std::formatter<std::remove_cvref_t<T>, char>` exists): `Format("{}", value)`.
- **Non-formattable types** (structs, byte views): a generic placeholder `<value@0x...>` so the framework stays compilable for opaque types.

Predefined specializations and overloads: `bool` (`true`/`false`), `char`, `StringView`, `p::String`, `const char*` (`(null)` for null), `std::string`, and any pointer type (`0xADDR`).

Extend for your own types by specializing the template:

```cpp
template<>
inline String p::TestString<MyType>(const MyType& v)
{
    return Format("MyType({})", v.id);
}
```

Prefer `StringView` overloads over `String` so views and literals both work without allocation.

### `TestSettings`

```cpp
struct TestSettings
{
    StringView only;             // Run only describes/its containing substring.
    StringView skip;             // Skip describes/its containing substring.
    bool dryRun           = false;  // Report full tree as SKIPPED, run nothing.
    bool breakOnFailure   = false;  // Stop the test run on the first failing test.
    bool useColor         = true;   // Colorized output.
    bool reportTiming     = false;  // Report per-test timing information.
    TTypeId<ITestReporter> reporter; // Reporter id; default = SpecReporter.
};
```

Process exit code: `0` when every test passed, `1` otherwise.

### Failure Reporting

- Failures are deferred: per-test assertions only update a per-test buffer (`file:line: msg`); the reporter flushes them at the end of the run.
- Each failed test contributes one entry to the global `failures` list, formatted as `context.test:\n<details>\n`.
- Failure messages are written to **stdout** (not stderr) so test-tooling parsers that follow bandit's convention (e.g. the Bandit VSCode adapter) keep working.
