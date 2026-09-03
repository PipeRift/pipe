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
		TestGroup root{"", {}, {}, {}, {}};

		// Pointer into `root.groups` for the currently-adding group.
		TestGroup* currentGroup = nullptr;
		int failedTests  = 0;
		int runTests     = 0;
		int skippedTests = 0;
	}    // namespace


	void Spec(StringView name, std::function<void()> fn)
	{
		TestGroup group;
		group.name       = String{name};
		group.beforeEach = nullptr;
		group.afterEach  = nullptr;
		root.groups.Add(Move(group));
		TestGroup* groupPtr = &root.groups.Last();
		currentGroup        = groupPtr;
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
		currentGroup->groups.Add(Move(group));
		TestGroup* prevGroup = currentGroup;
		currentGroup         = &currentGroup->groups.Last();
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
		currentGroup->tests.Add(Move(test));
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
		currentGroup->tests.Add(Move(test));
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

		static void RunNested(TestGroup& group,
		    TArray<std::function<void()>>& beforeHooks,
		    TArray<std::function<void()>>& afterHooks)
		{
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
					++failedTests;
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

		Info("PipeTests: {} group(s) registered.", root.groups.Size());
		TArray<std::function<void()>> beforeHooks;
		TArray<std::function<void()>> afterHooks;
		RunNested(root, beforeHooks, afterHooks);

		Info("PipeTests complete: {} run, {} passed, {} failed, {} skipped.",
		    runTests, runTests - failedTests, failedTests, skippedTests);

		return failedTests == 0 ? 0 : 1;
	}
}    // namespace p
