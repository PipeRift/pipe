# PipeTests Framework Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a Pipe-native test framework (`PipeTests` module) that mirrors Bandit's structure with imgui-style global context, used by both PipeTests and RiftTests.

**Architecture:** A new `PipeTests` module in the Pipe submodule (`Include/PipeTests.h` + `Src/PipeTests.cpp`) built as a **separate CMake library target** (never compiled into the runtime `Pipe` library). Global registration cursor tracks the current test group as functions are called. `Expect(value)` returns a fluent matcher. `p::RunTests(argc, argv)` runs the suite. Existing Bandit tests are NOT migrated and Bandit is NOT removed until the final task.

**Tech Stack:** C++20, CMake 3.26+, no exceptions, no RTTI (`-fno-rtti`). Pipe core types: `StringView`, `String`, `TArray`, `std::function`, `p::Format`, `p::Info/Warning/Error`.

## Global Constraints

- Namespace is `p`; user adds `using namespace p;`.
- Coding style: CamelCase functions, `camelBack` parameters/variables, tabs, 100-col limit, `.clang-format` (Microsoft base). Comments only where code is insufficient.
- No exceptions, no RTTI project-wide.
- Prefer `StringView` over `String`.
- Minimal templates; C-like C++.
- Do NOT modify or remove Bandit or existing `*spec.cpp` files until the final task (Task 6).
- Build/test commands:
  - Configure: `cmake -S . -B Build`
  - Build: `cmake --build Build --config Release`
  - Test: `cd Build && ctest --output-on-failure -j2 -C Release`
- All Pipe work happens in the Pipe submodule directory `Extern/Pipe` (branch `feature/pipe-tests`). Run git commands from inside `Extern/Pipe`.

---

### Task 1: Add `PipeTests` library target and unconditional build

**Files:**
- Modify: `Extern/Pipe/CMakeLists.txt` (Pipe library block ~lines 50-81; add new target after line 81)

**Interfaces:**
- Consumes: existing `Pipe` library target.
- Produces: CMake target `PipeTests` (linkable by other targets), available unconditionally (regardless of `PIPE_BUILD_TESTS`).

- [ ] **Step 1: Add the `PipeTests` library target**

Append after the `Pipe` library block in `Extern/Pipe/CMakeLists.txt` (after line 81, before the `PIPE_BUILD_TESTS` block):

```cmake
################################################################################
#   PipeTests (test framework library, not part of the runtime Pipe library)

add_library(PipeTests STATIC Src/PipeTests.cpp)
add_library(Pipe::TestsLib ALIAS PipeTests)
pipe_target_define_platform(PipeTests)
target_include_directories(PipeTests PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>)
pipe_target_enable_CPP20(PipeTests)
pipe_target_disable_rtti(PipeTests PRIVATE)
pipe_target_shared_output_directory(PipeTests)
target_link_libraries(PipeTests PUBLIC Pipe)
```

Note: `Src/PipeTests.cpp` does not exist yet; CMake will fail until Task 2 creates it.

- [ ] **Step 2: Ensure `Src/PipeTests.cpp` is excluded from the `Pipe` library glob**

The `Pipe` library compiles `Src/*.cpp` via `file(GLOB_RECURSE PIPE_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.c)` (line 65). `PipeTests.cpp` in `Src/` would be globbed into `Pipe`. Since the git repo does not track glob output, verify the exclusion after Task 2 by confirming the `Pipe` target does not include `PipeTests.cpp` (build command in Task 2 will confirm).
**If needed**: remove `Src/PipeTests.cpp` match from the glob by excluding subdirectory — glob includes it. To keep `PipeTests.cpp` out of `Pipe`, place it under a subdirectory instead: put the implementation at `Src/Tests/PipeTests.cpp` (not `Src/PipeTests.cpp`), and point the `PipeTests` target at `Src/Tests/PipeTests.cpp`. The `Pipe` glob `Src/*.cpp` (non-recursive at top level only matches `PipeTests.cpp` if directly in `Src/`; the actual glob is `GLOB_RECURSE ... Src/*.cpp` which is recursive and WILL pick up `Src/Tests/PipeTests.cpp`).

**Decision (must-follow):** Place the implementation at `Src/Tests/PipeTests.cpp` and exclude the `Src/Tests` directory from the `Pipe` source glob. Modify the `Pipe` glob (line 65) to exclude the `PipeTests` implementation:

```cmake
file(GLOB_RECURSE PIPE_SOURCE_FILES CONFIGURE_DEPENDS Src/*.cpp Src/*.c)
list(FILTER PIPE_SOURCE_FILES EXCLUDE REGEX ".*/Src/Tests/.*")
target_sources(Pipe PRIVATE ${PIPE_SOURCE_FILES})
```

Then the `PipeTests` target in this task uses `Src/Tests/PipeTests.cpp`:

```cmake
add_library(PipeTests STATIC Src/Tests/PipeTests.cpp)
```

- [ ] **Step 3: Configure + build (may fail until Task 2 creates the source)**

Run (from `Extern/Pipe`):
```
cmake -S . -B Build
cmake --build Build --config Release
```
Expected: fails only because `Src/Tests/PipeTests.cpp` (and `Include/PipeTests.h`) do not exist yet. This is acceptable mid-plan; the target is created and validated in Task 2.

- [ ] **Step 4: Commit**

```bash
git add CMakeLists.txt
git commit -m "build: add PipeTests library target"
```

---

### Task 2: `PipeTests.h` public header — registration functions

**Files:**
- Create: `Extern/Pipe/Include/PipeTests.h`

**Interfaces:**
- Consumes: `Pipe/Core/Log.h` (for error logging), `StringView.h`.
- Produces (used by Tasks 3-6):
  - `void Spec(StringView name, std::function<void()> fn)`
  - `void Spec(std::function<void()> fn)` (nameless)
  - `void Describe(StringView name, std::function<void()> fn)`
  - `void It(StringView name, std::function<void()> fn)`
  - `void XIt(StringView name, std::function<void()> fn)`
  - `void BeforeEach(std::function<void()> fn)`
  - `void AfterEach(std::function<void()> fn)`
  - `int RunTests(int argc, char** argv)`

- [ ] **Step 1: Declare the registration API**

Create `Extern/Pipe/Include/PipeTests.h`:

```cpp
// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Core/StringView.h"

#include <functional>


namespace p
{
	/**
	 * Test framework for Pipe and Rift.
	 * Imgui-style global context: registration functions act on a current group.
	 * Spec is self-registering; Describe/It/BeforeEach/AfterEach attach to the
	 * current group as functions are called.
	 */

	// Self-registering top-level. Spec(name, fn) also opens a first group named `name`.
	void Spec(StringView name, std::function<void()> fn);
	// Nameless top-level (like go_bandit); use Describe inside fn.
	void Spec(std::function<void()> fn);

	// Nested group. Only valid inside a Spec; otherwise logs an error and ignores.
	void Describe(StringView name, std::function<void()> fn);
	// Register a runnable test in the current group.
	void It(StringView name, std::function<void()> fn);
	// Register a disabled test; never run.
	void XIt(StringView name, std::function<void()> fn);
	// Setup hook attached to the current group.
	void BeforeEach(std::function<void()> fn);
	// Teardown hook attached to the current group.
	void AfterEach(std::function<void()> fn);

	int RunTests(int argc, char** argv);
};    // namespace p
```

- [ ] **Step 2: Commit**

```bash
git add Include/PipeTests.h
git commit -m "feat: declare PipeTests registration API"
```

---

### Task 3: `PipeTests.cpp` — registry, cursor, runner (skip + summary)

**Files:**
- Create: `Extern/Pipe/Src/Tests/PipeTests.cpp`

**Interfaces:**
- Consumes: `PipeTests.h`, `Pipe.h`, `Pipe/Core/Log.h`, `PipeStrings.h`, `StringView.h`, `TArray`.
- Produces: implementation of `Spec`, `Describe`, `It`, `XIt`, `BeforeEach`, `AfterEach`, `RunTests`. Matcher `Expect` is a separate task (Task 4); until then `It` bodies cannot assert.

The runner must support:
- Building a registered tree of groups and tests.
- Running each test, invoking `BeforeEach`/`AfterEach` hooks of the enclosing groups (outer → Test beforeEach first, test body, then AfterEach in reverse-within-group order).
- Skipping `XIt` tests (counted as skipped, not failures).
- Printing pass/fail/skip counts and failed test full names/locations.
- Returning `0` on success, non-zero if any test failed.

- [ ] **Step 1: Implement the registry data structure**

```cpp
// Copyright 2015-2026 Piperift. All Rights Reserved.

#ifndef P_OVERRIDE_NEWDELETE
	#define P_OVERRIDE_NEWDELETE 1
#endif
#if P_OVERRIDE_NEWDELETE
	#include "PipeNewDelete.h"
#endif

#include "PipeTests.h"
#include "Pipe.h"
#include "Pipe/Core/Log.h"
#include "PipeStrings.h"

#include <cstdio>


namespace p
{
	namespace
	{
		struct TestCase
		{
			String name;
			std::function<void()> body;
			bool skip = false;
		};

		struct TestGroup
		{
			String name;
			TArray<TestGroup> groups;      // nested describes
			TArray<TestCase> tests;         // its
			std::function<void()> beforeEach;
			std::function<void()> afterEach;
		};

		// Entire registered suite (treat as a single virtual root group).
		TestGroup root{"", nullptr, {}, {}, {}, {}};

		// Pointer into `root.groups` for the currently-adding group.
		TestGroup* currentGroup = nullptr;
		int failedTests = 0;
		int runTests = 0;
		int skippedTests = 0;
	}    // namespace
```

Note: `String` and `TArray` require `PipeStrings.h`/`PipeContainers.h` — included via `Pipe.h`? `Pipe.h` only includes `StringView.h` + `Export.h`. Include `PipeStrings.h` explicitly (done above). Ensure `PipeTests.cpp` links against Pipe (done in Task 1 via `target_link_libraries(PipeTests PUBLIC Pipe)`).

Because `TestGroup` contains `TArray<TestGroup>`, `TArray` must be usable with incomplete types or we must define nested groups via `std::vector`. Since Pipe disallows `std::vector` in headers but this is a `.cpp`, use `TArray<TestGroup>`. If `TArray<TestGroup>` fails to compile due to incomplete type during the initial member declaration, add a forward `struct TestGroup;` before `TestGroup` and store `TArray<TestGroup>`. Prefer keeping the above form; adjust only if the compiler requires it.

- [ ] **Step 2: Implement registration functions**

```cpp
	void Spec(StringView name, std::function<void()> fn)
	{
		TestGroup group;
		group.name = String{name};
		root.groups.Emplace(Move(group));
		TestGroup* groupPtr = &root.groups.Back();
		groupPtr->beforeEach = nullptr;
		groupPtr->afterEach = nullptr;
		currentGroup = groupPtr;
		fn();
		currentGroup = nullptr;
	}

	void Spec(std::function<void()> fn)
	{
		currentGroup = &root;
		fn();
		currentGroup = nullptr;
	}

	void Describe(StringView name, std::function<void()> fn)
	{
		if (!currentGroup)
		{
			Error("PipeTests: Describe('{}') called outside a Spec. Ignoring.", name);
			return;
		}

		TestGroup group;
		group.name = String{name};
		currentGroup->groups.Emplace(Move(group));
		TestGroup* prevGroup = currentGroup;
		currentGroup = &currentGroup->groups.Back();
		fn();
		currentGroup = prevGroup;
	}

	void It(StringView name, std::function<void()> fn)
	{
		if (!currentGroup)
		{
			Error("PipeTests: It('{}') called outside a Spec. Ignoring.", name);
			return;
		}
		TestCase test;
		test.name = String{name};
		test.body = fn;
		test.skip = false;
		currentGroup->tests.Emplace(Move(test));
	}

	void XIt(StringView name, std::function<void()> fn)
	{
		if (!currentGroup)
		{
			Error("PipeTests: XIt('{}') called outside a Spec. Ignoring.", name);
			return;
		}
		TestCase test;
		test.name = String{name};
		test.body = fn;
		test.skip = true;
		currentGroup->tests.Emplace(Move(test));
	}

	void BeforeEach(std::function<void()> fn)
	{
		if (!currentGroup)
		{
			Error("PipeTests: BeforeEach called outside a Spec. Ignoring.");
			return;
		}
		currentGroup->beforeEach = fn;
	}

	void AfterEach(std::function<void()> fn)
	{
		if (!currentGroup)
		{
			Error("PipeTests: AfterEach called outside a Spec. Ignoring.");
			return;
		}
		currentGroup->afterEach = fn;
	}
```

Note: The nameless `Spec(fn)` sets `currentGroup = &root`, which is a group with no name/beforeEach/afterEach and whose `groups`/`tests` are unused (it acts as a namespace root). Describe/It add into `root.groups`/`root.tests` directly. This matches bandit's `go_bandit` behavior. The `Error(StringView, Args...)` overload exists in `Log.h` (verified).

Verify the exact `TArray` API names: Pipe's `TArray` uses `Emplace`, `Push`, `Back`, `Size`, `Pop`. `Emplace` and `Back` are used in the code above. If the exact member names differ (e.g. `Add` instead of `Emplace`), adjust to Pipe's actual API shown in `Extern/Pipe/Include/PipeContainers.h` (line 768 `struct TArray`). Double-check `root{"", nullptr, {}, {}, {}, {}}` aggregate init is valid for the struct's member order: `name`, `groups`, `tests`, `beforeEach`, `afterEach`. Reorder the init to match the declared order:

```cpp
TestGroup root{"", {}, {}, {}, {}};
```

- [ ] **Step 3: Implement the runner**

```cpp
	static String FullName(const TestGroup& group, const TestCase& test)
	{
		// Build "SpecName.SubGroup.TestName" for reporting. Root has empty name.
		String result;
		if (!group.name.empty())
		{
			result += group.name;
			result += ".";
		}
		result += test.name;
		return result;
	}

	static void RunTest(const TestCase& test, const TestGroup& group)
	{
		if (test.skip)
		{
			++skippedTests;
			return;
		}

		++runTests;

		// Run the enclosing beforeEach hooks (outer groups first is handled by
		// recursion in RunGroup; here only the single group-level beforeEach
		// applies. For nested groups, RunTest is called with the innermost
		// group; BeforeEach hooks of outer groups are collected in RunRecogn.
		if (group.beforeEach)
		{
			group.beforeEach();
		}

		bool passed = true;
		try
		{
			test.body();
		}
		catch (...)
		{
			passed = false;
			Error("PipeTests: test failed by exception");
		}

		if (group.afterEach)
		{
			group.afterEach();
		}

		if (passed)
		{
			Info("  [PASS] {}", FullName(group, test));
		}
		else
		{
			++failedTests;
			Error("  [FAIL] {}", FullName(group, test));
		}
	}
```

Note on nested groups & BeforeEach: The above only runs the innermost group's beforeEach/afterEach. To match bandit (which runs BeforeEach of **all** enclosing groups outer→inner, then test, then AfterEach inner→outer), the runner must recurse. Implement `RunGroup` to: for each nested group, call `RunGroup` (which itself runs that group's `beforeEach` and `afterEach` around its tests AND its nested groups); for each test, run it. To run the contained `beforeEach` for nested recursion, structure as:

```cpp
	static void RunGroup(TestGroup& group)
	{
		for (TestGroup& sub : group.groups)
		{
			RunGroup(sub);
		}
		for (TestCase& test : group.tests)
		{
			// Manual hook application with proper nesting depth handled below.
			RunTest(test, group);
		}
	}
```

To correctly nest hooks across levels, thread a stack: pass a `TArray<std::function<void()>>&` of active beforeEach hooks and a matching afterEach stack. Simplest correct approach that matches bandit semantics:

```cpp
	static void RunNested(TestGroup& group,
	    TArray<std::function<void()>>& beforeHooks,
	    TArray<std::function<void()>>& afterHooks)
	{
		if (group.beforeEach)
		{
			beforeHooks.Emplace(group.beforeEach);
		}
		if (group.afterEach)
		{
			afterHooks.Emplace(group.afterEach);
		}

		for (TestGroup& sub : group.groups)
		{
			RunNested(sub, beforeHooks, afterHooks);
		}

		for (TestCase& test : group.tests)
		{
			if (test.skip)
			{
				++skippedTests;
				continue;
			}
			++runTests;

			for (auto& hook : beforeHooks)
			{
				hook();
			}

			bool passed = true;
			try
			{
				test.body();
			}
			catch (...)
			{
				passed = false;
				Error("PipeTests: test failed by exception: {}", FullName(group, test));
			}

			for (sizet i = afterHooks.Size(); i > 0; --i)
			{
				afterHooks[i - 1]();
			}

			if (passed)
			{
				Info("  [PASS] {}", FullName(group, test));
			}
			else
			{
				++failedTests;
				Error("  [FAIL] {}", FullName(group, test));
			}
		}

		if (group.beforeEach)
		{
			beforeHooks.Pop();
		}
		if (group.afterEach)
		{
			afterHooks.Pop();
		}
	}

	int RunTests(int argc, char** argv)
	{
		(void)argc;    // kept for future --filter support
		(void)argv;

		Info("PipeTests: {} group(s) registered.", root.groups.Size());
		TArray<std::function<void()>> beforeHooks;
		TArray<std::function<void()>> afterHooks;
		RunNested(root, beforeHooks, afterHooks);

		Info("PipeTests complete: {} run, {} passed, {} failed, {} skipped.",
		    runTests, runTests - failedTests, failedTests, skippedTests);

		return failedTests == 0 ? 0 : 1;
	}
```

Use the `RunNested` version (correct nested BeforeEach/AfterEach semantics). Verify `TArray` supports `Emplace(std::function)` (it stores by value; `std::function` is default-constructible and move-assignable — fine). `Pop()` removes last element.

- [ ] **Step 4: Configure + build**

Run (from `Extern/Pipe`):
```
cmake -S . -B Build
cmake --build Build --config Release
```
Expected: target `PipeTests` builds, `Pipe` library does NOT include `PipeTests.cpp`. If `/Src/Tests/` is still picked up by `Pipe`'s glob, fix the `list(FILTER ...)` exclusion (Task 1) and rebuild.

- [ ] **Step 5: Commit**

```bash
git add Src/Tests/PipeTests.cpp
git commit -m "feat: add PipeTests registry and runner"
```

---

### Task 4: Self-test the framework (small new tests, Bandit untouched)

**Files:**
- Create: `Extern/Pipe/Tests/PipeTests/PipeTests.spec.cpp`
- Create: `Extern/Pipe/Tests/PipeTests/main.cpp`

**Interfaces:**
- Consumes: `PipeTests.h`, `Pipe.h`, `p::Expect` (Task 5). To avoid depending on Task 5, this task's runner can use a temporary assertion until Task 5 lands. The spec file uses the framework's own `Spec/Describe/It/XIt/BeforeEach/AfterEach` + a minimal in-test check via a temporary macro defined in this file's `main.cpp` (or add `Expect` here as a stub returning `void`). Simpler: implement this task to compile against the header, but **defer the actual `Expect` matcher to Task 5 and add assertions in Task 5**. To keep the framework testable now, use a tiny local macro:

```cpp
#define CHECK_TRUE(x) do { if (!(x)) { p::Error("CHECK_TRUE failed: {}", #x); } } while (0)
```

- Produces: a second test executable `PipeTestsSelf` registered in CTest, proving the framework runs alongside the untouched Bandit suite.

- [ ] **Step 1: Create the self-test spec + runner**

`Extern/Pipe/Tests/PipeTests/main.cpp`:

```cpp
// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeNewDelete.h>

#include <Pipe.h>
#include <PipeTests.h>


int main(int argc, char* argv[])
{
	p::Initialize();
	int result = p::RunTests(argc, argv);
	p::Shutdown();
	return result;
}
```

`Extern/Pipe/Tests/PipeTests/PipeTests.spec.cpp`:

```cpp
// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTests.h>

#include <functional>

using namespace p;

static int beforeEachCount = 0;
static int afterEachCount = 0;
static int topTestResult = 0;


Spec("PipeTests", []() {
	BeforeEach([]() { ++beforeEachCount; });
	AfterEach([]() { ++afterEachCount; });

	Describe("Basics", []() {
		It("Registers and runs", []() { topTestResult = 42; });
		XIt("Is skipped", []() { topTestResult = -1; });
	});
});


// NOTE: assertions are added in Task 5 once Expect() exists.
```

- [ ] **Step 2: Wire a separate CTest target**

`Extern/Pipe/Tests/CMakeLists.txt` — add after the existing `PipeTests` executable block, **without modifying** the existing Bandit-based target or its `--reporter=spec`:

```cmake
# PipeTests self-test (uses the new framework). Bandit-based suite remains unchanged.
add_executable(PipeTestsSelf EXCLUDE_FROM_ALL src_placeholder_main)
```

Because the existing `PipeTests/CMakeLists.txt` uses `file(GLOB_RECURSE ...)` and adds its own `PipeTests` executable, adding a second executable in the same glob would collide (two `main.cpp`). Instead, add a **separate subdirectory** `Tests/PipeTests/` with its own `CMakeLists.txt`:

Create `Extern/Pipe/Tests/PipeTests/CMakeLists.txt`:

```cmake
add_executable(PipeTestsSelf PipeTests.spec.cpp main.cpp)
pipe_target_define_platform(PipeTestsSelf)
pipe_target_enable_CPP20(PipeTestsSelf)
pipe_target_disable_rtti(PipeTestsSelf PRIVATE)
pipe_target_shared_output_directory(PipeTestsSelf)
target_link_libraries(PipeTestsSelf PUBLIC PipeTests Pipe)
add_test(NAME PipeTestsSelf COMMAND $<TARGET_FILE:PipeTestsSelf>)
```

And in `Extern/Pipe/Tests/CMakeLists.txt`, add `add_subdirectory(PipeTests)` at the end (the parent glob will NOT recurse into `PipeTests` because the existing glob is `GLOB_RECURSE *.cpp *.h *.hpp` which WILL pick up the new `PipeTests.spec.cpp` and `main.cpp` into the Bandit-based `PipeTests` executable — causing duplicate `main()` and redefinition). To prevent that:

- Change the parent glob to limit scope, OR
- Place the self-test under a path not matched by the parent glob, OR
- The cleanest: **exclude the `PipeTestsSelf` sources from the Bandit glob** by listing them and filtering. Simplest robust approach given CMake: rename the self-test directory so the `GLOB_RECURSE` doesn't include it is not possible (it globs everything). Instead, filter in `Tests/CMakeLists.txt`:

```cmake
file(GLOB_RECURSE TESTS_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h *.hpp)
list(FILTER TESTS_SOURCE_FILES EXCLUDE REGEX ".*/PipeTests/.*")
add_executable(PipeTests ${TESTS_SOURCE_FILES})
# ... existing target_link_libraries(PipeTests PUBLIC Pipe Bandit) unchanged ...
add_subdirectory(PipeTests)
```

This keeps the Bandit-based `PipeTests` exe only on non-`PipeTests/` sources, and adds `PipeTestsSelf` from the subdirectory. Verify the `main()` symbol is only defined once in the Bandit exe.

- [ ] **Step 3: Configure + build + run**

Run (from `Extern/Pipe`):
```
cmake -S . -B Build
cmake --build Build --config Release
ctest --test-dir Build --output-on-failure
```
Expected: `PipeTestsSelf` appears in CTest output with `0 run, 0 passed` (no assertions yet) and reports the skipped `XIt` count (1 skipped) and 1 pass. The Bandit suite `PipeTests` also still runs (`--reporter=spec`) with its original tests.

- [ ] **Step 4: Commit**

```bash
git add Tests/PipeTests
git commit -m "test: add PipeTests self-test suite"
```

---

### Task 5: `Expect` fluent matcher + extensible formatter

**Files:**
- Modify: `Extern/Pipe/Include/PipeTests.h`
- Modify: `Extern/Pipe/Src/Tests/PipeTests.cpp`

**Interfaces:**
- Consumes: `PipeTests.h` registration API (Task 2/3), Pipe `Format`, `StringView`, `Number` concept (`TypeTraits.h`).
- Produces: `Expect(value)` matcher returned by `p::Expect(value)` with methods `ToEqual`, `ToNotEqual`, `ToBeLess`, `ToBeLessOrEqual`, `ToBeGreater`, `ToBeGreaterOrEqual`, `ToBeTrue`, `ToBeFalse`, `ToContain`, `ToNotContain`. Failure prints `file:line` + actual/expected via an extensible `ToString`-style hook (`p::TestString`).

- [ ] **Step 1: Add the formatter hook and matcher to `PipeTests.h`**

Append to `PipeTests.h`:

```cpp
	// Extensible value-to-string hook for failure messages.
	// Specialize for user types. Default handles numbers and string views.
	template<typename T>
	inline String TestString(const T& value);

	namespace details
	{
		// Format failure message from file:line + description.
		P_API void Fail(const char* file, sizet line, StringView message);
	}    // namespace details
```

Implement `TestString` in the header (template):

```cpp
	template<typename T>
	inline String TestString(const T& value)
	{
		return Format("{}", value);
	}

	template<>
	inline String TestString<bool>(const bool& value)
	{
		return value ? String{"true"} : String{"false"};
	}

	template<>
	inline String TestString<char>(const char& value)
	{
		return String{value};
	}

	inline String TestString(const StringView value)
	{
		return String{value};
	}

	inline String TestString(const char* value)
	{
		return value ? String{value} : String{"(null)"};
	}
```

Note: `Format("{}", value)` requires `value` be formattable; Pipe's `STDFormat.h` and `PipeStrings.h` provide `std::format` for arithmetic and `String`/`StringView`/`const char*` (StringView formats as string view). Confirm `StringView` has a `std::formatter`; Pipe formats strings via `std::format` — check `PipeStrings.h`/`Pipe/Core/STDFormat.h` supplies a formatter for `StringView` and `String`. If `Format("{}", StringView)` does not compile, add an overload:

```cpp
	inline String TestString(const String& value)
	{
		return String{value};
	}
```

(Add if needed; Strings.format uses `std::vformat_to` which requires appropriate formatters.)

Now the matcher class:

```cpp
	// ---- fluent assertion ----
	template<typename Actual>
	class ExpectValue
	{
	public:
		ExpectValue(const Actual& value, const char* file, sizet line)
		    : value(value)
		    , file(file)
		    , line(line)
		{}

		void ToEqual(const Actual& expected) const
		{
			if (!(value == expected))
			{
				details::Fail(file, line, Format(
				    "Expected {} to equal {}", TestString(value), TestString(expected)));
			}
		}

		void ToNotEqual(const Actual& expected) const
		{
			if (!(value != expected))
			{
				details::Fail(file, line, Format(
				    "Expected {} to not equal {}", TestString(value), TestString(expected)));
			}
		}

		void ToBeLess(const Actual& other) const
		{
			if (!(value < other))
			{
				details::Fail(file, line, Format(
				    "Expected {} to be less than {}", TestString(value), TestString(other)));
			}
		}

		void ToBeLessOrEqual(const Actual& other) const
		{
			if (!(value <= other))
			{
				details::Fail(file, line, Format(
				    "Expected {} to be less or equal to {}", TestString(value), TestString(other)));
			}
		}

		void ToBeGreater(const Actual& other) const
		{
			if (!(value > other))
			{
				details::Fail(file, line, Format(
				    "Expected {} to be greater than {}", TestString(value), TestString(other)));
			}
		}

		void ToBeGreaterOrEqual(const Actual& other) const
		{
			if (!(value >= other))
			{
				details::Fail(file, line, Format(
				    "Expected {} to be greater or equal to {}", TestString(value), TestString(other)));
			}
		}

		void ToBeTrue() const
		{
			if (!value)
			{
				details::Fail(file, line, "Expected value to be true");
			}
		}

		void ToBeFalse() const
		{
			if (value)
			{
				details::Fail(file, line, "Expected value to be false");
			}
		}

		void ToContain(const StringView sub) const
		{
			// Actual must be a string-like type.
			StringView view{value};
			if (Strings::Find(view, sub) == StringView::npos)
			{
				details::Fail(file, line, Format(
				    "Expected {} to contain {}", TestString(value), TestString(sub)));
			}
		}

		void ToNotContain(const StringView sub) const
		{
			StringView view{value};
			if (Strings::Find(view, sub) != StringView::npos)
			{
				details::Fail(file, line, Format(
				    "Expected {} to not contain {}", TestString(value), TestString(sub)));
			}
		}

	private:
		const Actual& value;
		const char* file;
		sizet line;
	};
```

Note: `ToBeTrue/ToBeFalse` require `value` convertible to bool (works for bool and pointer/integer). For `ToContain`, `StringView view{value}` requires `value` convertible to `StringView` (works for `StringView`, `const char*`, `std::string_view`, `String`). For `Expect(...).ToContain("acid")` with a `String`/`const char*` actual, `StringView view{value}` must be constructible. Confirm `String` is constructible to `StringView` (it exposes a `View` alias and an operator/conversion). If `String` does not implicitly convert, add `StringView{value.c_str(), value.size()}`.

Finally the entry macro/function:

```cpp
	// Returns a matcher bound to file/line for reporting.
	template<typename T>
	ExpectValue<T> Expect(const T& value, const char* file = __FILE__, sizet line = __LINE__)
	{
		return ExpectValue<T>(value, file, line);
	}
```

Note: capturing `__FILE__`/`__LINE__` at the `Expect(...)` call gives the caller's location. This is a plain template returning a matcher; no macro needed. This matches the fluent `Expect(value).ToEqual(4)` usage.

- [ ] **Step 2: Implement `details::Fail` in the `.cpp`**

Append to `PipeTests.cpp`:

```cpp
	namespace details
	{
		void Fail(const char* file, sizet line, StringView message)
		{
			Error("PipeTests: {}:{}: {}", file, line, message);
		}
	}    // namespace details
```

`Error` is the Pipe log function (from `Log.h`, already included). This reports a failure inline; the runner counts it as a failure (Task 3 `RunNested` sets `passed=false` only on exception). **Critical:** `Fail` must mark the current test failed. Currently `RunNested` only flips `passed` on exception. Change the failure tracking: add a global `int currentTestFailed = 0;` plus `bool CurrentTestFailed()` accessor, OR have `Fail` set a global flag checked after `test.body()`. Implement:

In the anonymous namespace add:
```cpp
		int currentTestFailureCount = 0;
```
In `details::Fail`:
```cpp
		void Fail(const char* file, sizet line, StringView message)
		{
			Error("PipeTests: {}:{}: {}", file, line, message);
			++currentTestFailureCount;
		}
```
In `RunNested`, before running the body reset the count, after body if `currentTestFailureCount > 0` mark failed and reset:

```cpp
		currentTestFailureCount = 0;
		bool passed = true;
		try
		{
			test.body();
		}
		catch (...)
		{
			passed = false;
			Error("PipeTests: test failed by exception: {}", FullName(group, test));
		}
		passed = passed && (currentTestFailureCount == 0);
		if (!passed)
		{
			++failedTests;
			Error("  [FAIL] {}", FullName(group, test));
		}
		else
		{
			Info("  [PASS] {}", FullName(group, test));
		}
```

- [ ] **Step 3: Add real assertions to the self-test**

Update `Extern/Pipe/Tests/PipeTests/PipeTests.spec.cpp` to use `Expect`:

```cpp
#include <PipeTests.h>
#include <PipeStrings.h>

using namespace p;

Spec("PipeTests", []() {
	Describe("Expect", []() {
		It("ToEqual / ToNotEqual", []() {
			int value = 4;
			Expect(value).ToEqual(4);
			Expect(value).ToNotEqual(5);
		});
		It("Relational", []() {
			int value = 4;
			Expect(value).ToBeLess(5);
			Expect(value).ToBeLessOrEqual(4);
			Expect(value).ToBeGreater(3);
			Expect(value).ToBeGreaterOrEqual(4);
		});
		It("Booleans", []() {
			bool flag = true;
			Expect(flag).ToBeTrue();
			Expect(!flag).ToBeFalse();
		});
		It("Strings", []() {
			Expect("acidic").ToContain("acid");
			Expect(String{"hello"}).ToNotContain("world");
		});
		It("Equals int", []() {
			Expect(4).ToEqual(4);
		});
	});
});
```

(note: `Expect(value).ToBeTrue()` requires `value` be usable in `if (!value)`; bool works.)

- [ ] **Step 4: Build + run, confirm failure counts**

Run (from `Extern/Pipe`):
```
cmake --build Build --config Release
ctest --test-dir Build --output-on-failure -R PipeTestsSelf
```
Expected: `PipeTestsSelf` runs the `Expect` tests, all pass (except we also want to verify a deliberate failure is counted — optional: temporarily add `Expect(1).ToEqual(2);` to confirm the FAIL path, then remove).

Verify one failure is caught: temporarily add to a test `Expect(1).ToEqual(2);`, run, confirm `failed` count = 1 and exit non-zero, then remove it and re-run to confirm green.

- [ ] **Step 5: Commit**

```bash
git add Include/PipeTests.h Src/Tests/PipeTests.cpp Tests/PipeTests
git commit -m "feat: add Expect fluent matcher"
```

---

### Task 6: Remove Bandit + migrate existing tests (FINAL — only after Tasks 1-5 pass)

**Files:**
- Modify: `Extern/Pipe/Tests/CMakeLists.txt`
- Modify: `Extern/Pipe/Tests/main.cpp`
- Modify: `Extern/Pipe/Extern/CMakeLists.txt` (remove `Bandit`)
- Delete: `Extern/Pipe/Extern/Bandit/` (vended dir)
- Modify: all `Extern/Pipe/Tests/**/*.spec.cpp`
- Modify: `Extern/Pipe/Tests/PipeTests/CMakeLists.txt` (remove self-only scope if desired) — optional; keep separate target.
- Modify: `Tests/CMakeLists.txt` (Rift) and `Tests/*.spec.cpp` (Rift) in `D:\Projects\Piperift\rift`

**Interfaces:**
- Consumes: `PipeTests.h`, `p::RunTests` (Tasks 2-5).
- Produces: Bandit fully removed; both Pipe and Rift suites run on the native framework.

⚠️ **This task is intentionally LAST. Do not start it until Tasks 1-5 are complete and verified.**

- [ ] **Step 1: Migrate one reference spec file (Pipe)**

Convert `Extern/Pipe/Tests/Core/StringView.spec.cpp`:

Old:
```cpp
#include <bandit/bandit.h>
#include <Pipe/Core/StringView.h>
#include <PipeStrings.h>

using namespace snowhouse;
using namespace bandit;
using namespace p;

go_bandit([]()
{
	describe("Strings", []()
	{
		describe("StringView", []()
		{
			it("Can assign from literal", [&]()
			{
				StringView v{"Kiwi"};
				AssertThat(v, Equals("Kiwi"));
				AssertThat(v.size(), Equals(4));
			});
			// ... other tests ...
		});
	});
});
```

New:
```cpp
#include <PipeTests.h>
#include <Pipe/Core/StringView.h>
#include <PipeStrings.h>

using namespace p;

Spec("Strings", []()
{
	Describe("StringView", []()
	{
		It("Can assign from literal", []()
		{
			StringView v{"Kiwi"};
			Expect(v).ToEqual("Kiwi");
			Expect(v.size()).ToEqual(4);
		});
		// ... other tests converted similarly ...
	});
});
```

Transform rules (from the spec):
- `#include <bandit/bandit.h>` → `#include <PipeTests.h>`
- `using namespace snowhouse; using namespace bandit;` → remove both; keep `using namespace p;`
- `go_bandit([](){ describe("G", [](){ ...`) → `Spec("G", [](){ ...`  (drop the outer `go_bandit` extra nesting and one `describe` level; the top `Spec("Strings", ...)` replaces go_bandit+first describe)
- `describe(` → `Describe(`
- `it(` → `It(` (drop the `[&]` → `[]`; lambdas no longer need `&` capture since framework state is global)
- `xit(` → `XIt(`
- `before_each(` → `BeforeEach(`
- `after_each(` → `AfterEach(`
- `AssertThat(v, Equals(x))` → `Expect(v).ToEqual(x)`
- `AssertThat(v, !Equals(x))` → `Expect(v).ToNotEqual(x)`
- `AssertThat(x, Equals(true))` → `Expect(x).ToBeTrue()`
- `AssertThat(x, !Equals(true))` → `Expect(x).ToBeFalse()`
- `AssertThat(x, Equals(false))` → `Expect(x).ToBeFalse()`
- `AssertThat(x, Is().True())` → `Expect(x).ToBeTrue()`
- `AssertThat(x, Is().False())` → `Expect(x).ToBeFalse()`
- `AssertThat(v.size(), Equals(4u))` → `Expect(v.size()).ToEqual(4u)`

Important: the top-level transform. Bandit files use `go_bandit([](){ describe("Strings", [](){...}) });`. Our `Spec("Strings", [](){...})` handles the `describe` level directly, so replace the pair with a single `Spec("Strings", fn)` and inside use `Describe`/`It`. For files that use `go_bandit` with a single top describe, keep that one as `Spec` and drop the now-redundant `Describe` wrapper if present. Follow the reference conversion exactly.

- [ ] **Step 2: Build + run the migrated file only (green)**

Run (from `Extern/Pipe`):
```
cmake --build Build --config Release
```
Ensure `StringView.spec.cpp` id not picked up by the Bandit exe twice. The Pipe tests CMake glob (`GLOB_RECURSE *.cpp` in `Tests/CMakeLists.txt`) picks up all `spec.cpp` including the migrated one — but the Bandit exe will FAIL to compile the migrated file (it no longer includes bandit). **Must switch the whole `PipeTests` exe to the new framework now**, not incrementally. Therefore:

**Decision:** Because `Tests/CMakeLists.txt` globs all `spec.cpp` into one `PipeTests` exe, migration must flip the entire Pipe suite at once (not file-by-file) to keep it compiling. Steps 1-3 migrate ALL Pipe spec files in one pass, then build once. Verify the whole Pipe suite passes via the new framework.

- [ ] **Step 3: Migrate ALL remaining Pipe spec files**

Convert every `Extern/Pipe/Tests/**/*.spec.cpp` using the transform rules above. Remove bandit includes and namespaces, map to `Spec/Describe/It/XIt/BeforeEach/AfterEach` and `Expect`. Use `[]` (no `&` capture) for lambda bodies.

- [ ] **Step 4: Switch PipeTests exe to the new framework**

`Extern/Pipe/Tests/CMakeLists.txt`:
- Change `target_link_libraries(PipeTests PUBLIC Pipe Bandit)` → `target_link_libraries(PipeTests PUBLIC Pipe PipeTests)`
- Remove `--reporter=spec` from `add_test(...)`:
  `add_test(NAME PipeTests COMMAND $<TARGET_FILE:PipeTests>)`
- Remove the `list(FILTER ...)` exclusion added in Task 4 (restore the plain glob) so all spec files (including migrated ones) build into `PipeTests`.

`Extern/Pipe/Tests/main.cpp`: replace `int result = bandit::run(argc, argv);` with `int result = p::RunTests(argc, argv);`, and remove `#include <bandit/bandit.h>`. Keep `#include <PipeNewDelete.h>` and `p::Initialize`/`p::Shutdown`.

`Extern/Pipe/Tests/PipeTests/CMakeLists.txt`: keep the `PipeTestsSelf` target for framework self-checks, OR fold the self-test spec files into the main `PipeTests` glob (remove the separate subdirectory). Keep `PipeTestsSelf` as-is for now (harmless), unless the main glob re-includes its files. Since the main glob is `GLOB_RECURSE *.cpp` from `Tests/`, it WILL include `Tests/PipeTests/*.cpp` again → duplicate `main()`. So when restoring the plain glob in step 4, re-apply a filter to EXCLUDE `Tests/PipeTests/` from the main `PipeTests` exe (keep `PipeTestsSelf` as a separate target):

```cmake
file(GLOB_RECURSE TESTS_SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h *.hpp)
list(FILTER TESTS_SOURCE_FILES EXCLUDE REGEX ".*/PipeTests/.*")
add_executable(PipeTests ${TESTS_SOURCE_FILES})
```

Keep `add_subdirectory(PipeTests)` for `PipeTestsSelf`.

- [ ] **Step 5: Build + run full Pipe suite (green)**

Run (from `Extern/Pipe`):
```
cmake --build Build --config Release
ctest --test-dir Build --output-on-failure
```
Expected: `PipeTests` runs all migrated tests with names/locations under the new framework; `PipeTestsSelf` still passes. Bandit no longer referenced.

- [ ] **Step 6: Remove the Bandit dependency**

- `Extern/Pipe/Extern/CMakeLists.txt`: remove lines 6-7 (`add_library(Bandit INTERFACE)` + include dir).
- Delete `Extern/Pipe/Extern/Bandit/` directory.
- `git rm -r Extern/Bandit` (from `Extern/Pipe`).

- [ ] **Step 7: Migrate Rift tests + CMake**

In `D:\Projects\Piperift\rift`:
- `Tests/CMakeLists.txt`: `target_link_libraries(RiftTests PUBLIC RiftASTLib Bandit)` → `target_link_libraries(RiftTests PUBLIC RiftASTLib PipeTests)`. Rift links `PipeTests` from the Pipe submodule; ensure Rift's build reaches the `PipeTests` target (it is defined unconditionally in `Extern/Pipe/CMakeLists.txt` per Task 1).
- Convert Rift `Tests/Project.spec.cpp`, `Tests/AST/Statements.spec.cpp`, `Tests/AST/Expressions.spec.cpp`, `Tests/AST/Namespaces.spec.cpp` per the transform rules (uses `before_each`/`after_each` → `BeforeEach`/`AfterEach`, `AssertThat(result, Equals(true))` → `Expect(result).ToBeTrue()`, etc.). Replace `#include <bandit/bandit.h>` and `using namespace snowhouse/bandit`.

- [ ] **Step 8: Full project build + tests + format**

Run (from `D:\Projects\Piperift\rift`):
```
cmake --build Build --config Release
cd Build && ctest --output-on-failure -j2 -C Release
```
and format: `cmake --build Build --target ClangFormat`.

Expected: all green; no reference to Bandit anywhere in the build.

- [ ] **Step 9: Commit (Pipe) + Commit (Rift)**

```bash
# From Extern/Pipe
git add -A
git commit -m "test: replace Bandit with PipeTests framework"

# From D:\Projects\Piperift\rift (updated submodule pointer + Rift tests + CMake)
git add Extern/Pipe Tests CMakeLists.txt
git commit -m "test: use PipeTests framework in Rift tests"
```

Note: the Rift commit must record the new Pipe submodule hash (`git add Extern/Pipe`).

---

## Self-Review

**Spec coverage:**
- ✅ Native `PipeTests` module in Pipe tree (Task 1-3)
- ✅ Used by both PipeTests and RiftTests (Tasks 4, 6)
- ✅ Mirrors bandit structure `Spec/Describe/It/XIt/BeforeEach/AfterEach` (Tasks 2, 3, 6)
- ✅ `Expect` fluent matcher + extensible formatter (Task 5)
- ✅ Detailed `file:line` + actual/expected (Task 5 `details::Fail`)
- ✅ Bandit removed only at the very end (Task 6), Bandit coexists during dev (Tasks 1-5)
- ✅ No runtime burden on shipped `Pipe` lib (separate target, `Src/Tests/` excluded — Task 1)
- ✅ No `ToThrow` (no exceptions matchers) — confirmed
- ✅ `Describe` misuse = log + ignore (Task 3)
- ✅ `RunTests(int, char**)` (Tasks 2, 3)
- ✅ imgui-style global context, only `Spec` self-registers (Task 2, 3)

**Placeholder scan:** All steps carry concrete code. The `Expect` matcher uses `Format("{}", value)` which needs a `StringView` formatter — flagged with an explicit fallback overload if missing. Task adds a note to verify `TArray` member names and add `String` no implicit `StringView` conversion fallback. No TODO/TBD beyond explicit in-task verification notes.

**Type consistency:** `String`, `StringView`, `sizet`, `Number`, `TestString`, `ExpectValue<T>`, `details::Fail(file, line, message)`, `RunTests(int,char**)` used consistently across tasks.