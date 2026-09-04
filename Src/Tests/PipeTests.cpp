// Copyright 2015-2026 Piperift. All Rights Reserved.

#ifndef P_OVERRIDE_NEWDELETE
	#define P_OVERRIDE_NEWDELETE 1
#endif
#if P_OVERRIDE_NEWDELETE
	#include "PipeNewDelete.h"
#endif

#include "Pipe.h"
#include "Pipe/Core/Log.h"
#include "PipeStrings.h"
#include "PipeTests.h"


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
			TArray<TestGroup> groups;    // nested describes
			TArray<TestCase> tests;      // its
			std::function<void()> beforeEach;
			std::function<void()> afterEach;
		};

		// Entire registered suite (treat as a single virtual root group).
		struct RegistryState
		{
			TestGroup root{"", {}, {}, {}, {}};

			// Pointer into `root.groups` for the currently-adding group.
			TestGroup* currentGroup = nullptr;
			int failedTests         = 0;
			int runTests            = 0;
			int skippedTests        = 0;
		};

		// Function-local static: initialized on first use regardless of the
		// static-init order of other translation units, so a `Spec` registrar
		// defined in a separate TU can safely register during static init.
		RegistryState& State()
		{
			static RegistryState state;
			return state;
		}

		TestGroup*& CurrentGroup()
		{
			return State().currentGroup;
		}
	}    // namespace


	void Spec(StringView name, std::function<void()> fn)
	{
		RegistryState& state = State();
		TestGroup group;
		group.name       = String{name};
		group.beforeEach = nullptr;
		group.afterEach  = nullptr;
		state.root.groups.Add(Move(group));
		TestGroup* groupPtr = &state.root.groups.Last();
		state.currentGroup  = groupPtr;
		fn();
		state.currentGroup = nullptr;
	}

	void Spec(std::function<void()> fn)
	{
		RegistryState& state = State();
		state.currentGroup   = &state.root;
		fn();
		state.currentGroup = nullptr;
	}

	void Describe(StringView name, std::function<void()> fn)
	{
		TestGroup*& current = CurrentGroup();
		if (!current)
		{
			Error("PipeTests: Describe('{}') called outside a Spec. Ignoring.", name);
			return;
		}

		TestGroup group;
		group.name = String{name};
		current->groups.Add(Move(group));
		TestGroup* prevGroup = current;
		current              = &current->groups.Last();
		fn();
		current = prevGroup;
	}

	void It(StringView name, std::function<void()> fn)
	{
		TestGroup*& current = CurrentGroup();
		if (!current)
		{
			Error("PipeTests: It('{}') called outside a Spec. Ignoring.", name);
			return;
		}
		TestCase test;
		test.name = String{name};
		test.body = fn;
		test.skip = false;
		current->tests.Add(Move(test));
	}

	void XIt(StringView name, std::function<void()> fn)
	{
		TestGroup*& current = CurrentGroup();
		if (!current)
		{
			Error("PipeTests: XIt('{}') called outside a Spec. Ignoring.", name);
			return;
		}
		TestCase test;
		test.name = String{name};
		test.body = fn;
		test.skip = true;
		current->tests.Add(Move(test));
	}

	void BeforeEach(std::function<void()> fn)
	{
		TestGroup*& current = CurrentGroup();
		if (!current)
		{
			Error("PipeTests: BeforeEach called outside a Spec. Ignoring.");
			return;
		}
		current->beforeEach = fn;
	}

	void AfterEach(std::function<void()> fn)
	{
		TestGroup*& current = CurrentGroup();
		if (!current)
		{
			Error("PipeTests: AfterEach called outside a Spec. Ignoring.");
			return;
		}
		current->afterEach = fn;
	}


	namespace
	{
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

		static void RunNested(TestGroup& group, TArray<std::function<void()>>& beforeHooks,
		    TArray<std::function<void()>>& afterHooks)
		{
			RegistryState& state = State();
			if (group.beforeEach)
			{
				beforeHooks.Add(group.beforeEach);
			}
			if (group.afterEach)
			{
				afterHooks.Add(group.afterEach);
			}

			for (TestGroup& sub : group.groups)
			{
				RunNested(sub, beforeHooks, afterHooks);
			}

			for (TestCase& test : group.tests)
			{
				if (test.skip)
				{
					++state.skippedTests;
					continue;
				}
				++state.runTests;

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

				for (i32 i = afterHooks.Size(); i > 0; --i)
				{
					afterHooks[i - 1]();
				}

				if (passed)
				{
					Info("  [PASS] {}", FullName(group, test));
				}
				else
				{
					++state.failedTests;
					Error("  [FAIL] {}", FullName(group, test));
				}
			}

			if (group.beforeEach)
			{
				beforeHooks.RemoveLast();
			}
			if (group.afterEach)
			{
				afterHooks.RemoveLast();
			}
		}
	}    // namespace


	int RunTests(int argc, char** argv)
	{
		(void)argc;    // kept for future --filter support
		(void)argv;

		RegistryState& state = State();
		Info("PipeTests: {} group(s) registered.", state.root.groups.Size());
		TArray<std::function<void()>> beforeHooks;
		TArray<std::function<void()>> afterHooks;
		RunNested(state.root, beforeHooks, afterHooks);

		Info("PipeTests complete: {} run, {} passed, {} failed, {} skipped.", state.runTests,
		    state.runTests - state.failedTests, state.failedTests, state.skippedTests);

		return state.failedTests == 0 ? 0 : 1;
	}
}    // namespace p
