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
#include "PipeTest.h"


namespace p
{
	namespace
	{
		struct TestCase
		{
			String name;
			// Owning: bodies are stored until RunTests runs them.
			std::function<void()> body;
			bool skip = false;
		};

		struct TestDescribe
		{
			String name;
			TArray<TestDescribe> describes;    // nested describes
			TArray<TestCase> tests;            // its tests
			// Owning: hooks are stored until RunTests runs them.
			std::function<void()> beforeEach;
			std::function<void()> afterEach;
		};

		// Entire registered suite (treat as a single virtual root describe).
		struct TestContext
		{
			TestDescribe root{"", {}, {}, {}, {}};

			// Pointer into `root.describes` for the currently-adding describe.
			TestDescribe* currentDescribe     = nullptr;
			i32 failedTests                   = 0;
			i32 runTests                      = 0;
			i32 skippedTests                  = 0;
			i32 currentTestFailureCount       = 0;
		};

		// Function-local static: initialized on first use regardless of the
		// static-init order of other translation units, so a `Spec` call
		// at file scope in a separate TU can safely register during static init.
		TestContext& GetTestContext()
		{
			static TestContext context;
			return context;
		}

		TestDescribe*& CurrentDescribe()
		{
			return GetTestContext().currentDescribe;
		}
	}    // namespace


	namespace details
	{
		void Fail(const std::source_location& loc, StringView message)
		{
			Error("PipeTests: {}:{}: {}", loc.file_name(), loc.line(), message);
			++GetTestContext().currentTestFailureCount;
		}
	}    // namespace details


	void Spec(StringView name, TFunction<void()> fn)
	{
		TestContext& context = GetTestContext();
		TestDescribe describe;
		describe.name       = String{name};
		describe.beforeEach = nullptr;
		describe.afterEach  = nullptr;
		context.root.describes.Add(Move(describe));
		TestDescribe* describePtr = &context.root.describes.Last();
		context.currentDescribe   = describePtr;
		fn();
		context.currentDescribe = nullptr;
	}

	void Spec(TFunction<void()> fn)
	{
		TestContext& context   = GetTestContext();
		context.currentDescribe = &context.root;
		fn();
		context.currentDescribe = nullptr;
	}

	void Describe(StringView name, TFunction<void()> fn)
	{
		TestDescribe*& current = CurrentDescribe();
		if (!current)
		{
			Error("PipeTests: Describe('{}') called outside a Spec. Ignoring.", name);
			return;
		}

		TestDescribe describe;
		describe.name          = String{name};
		current->describes.Add(Move(describe));
		TestDescribe* prevDescribe = current;
		current                    = &current->describes.Last();
		fn();
		current = prevDescribe;
	}

	void It(StringView name, std::function<void()> fn)
	{
		TestDescribe*& current = CurrentDescribe();
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
		TestDescribe*& current = CurrentDescribe();
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
		TestDescribe*& current = CurrentDescribe();
		if (!current)
		{
			Error("PipeTests: BeforeEach called outside a Spec. Ignoring.");
			return;
		}
		current->beforeEach = fn;
	}

	void AfterEach(std::function<void()> fn)
	{
		TestDescribe*& current = CurrentDescribe();
		if (!current)
		{
			Error("PipeTests: AfterEach called outside a Spec. Ignoring.");
			return;
		}
		current->afterEach = fn;
	}


	namespace
	{
		static String FullName(const TestDescribe& describe, const TestCase& test)
		{
			// Build "SpecName.SubDescribe.TestName" for reporting. Root has empty name.
			String result;
			if (!describe.name.empty())
			{
				result += describe.name;
				result += ".";
			}
			result += test.name;
			return result;
		}

		static bool MatchesFilter(StringView fullName, StringView filter)
		{
			return filter.empty() || Strings::Contains(fullName, filter);
		}

		static void RunNested(TestDescribe& describe, TArray<std::function<void()>>& beforeHooks,
		    TArray<std::function<void()>>& afterHooks, StringView filter)
		{
			TestContext& context = GetTestContext();
			if (describe.beforeEach)
			{
				beforeHooks.Add(describe.beforeEach);
			}
			if (describe.afterEach)
			{
				afterHooks.Add(describe.afterEach);
			}

			for (TestDescribe& sub : describe.describes)
			{
				RunNested(sub, beforeHooks, afterHooks, filter);
			}

			for (TestCase& test : describe.tests)
			{
				if (test.skip)
				{
					++context.skippedTests;
					continue;
				}
				if (!MatchesFilter(FullName(describe, test), filter))
				{
					continue;
				}
				++context.runTests;

				for (auto& hook : beforeHooks)
				{
					hook();
				}

				context.currentTestFailureCount = 0;
				bool passed = true;
				try
				{
					test.body();
				}
				catch (...)
				{
					passed = false;
					Error("PipeTests: test failed by exception: {}", FullName(describe, test));
				}
				passed = passed && (context.currentTestFailureCount == 0);

				for (i32 i = afterHooks.Size(); i > 0; --i)
				{
					afterHooks[i - 1]();
				}

				if (passed)
				{
					Info("  [PASS] {}", FullName(describe, test));
				}
				else
				{
					++context.failedTests;
					Error("  [FAIL] {}", FullName(describe, test));
				}
			}

			if (describe.beforeEach)
			{
				beforeHooks.RemoveLast();
			}
			if (describe.afterEach)
			{
				afterHooks.RemoveLast();
			}
		}
	}    // namespace


	int RunTests(const TestSettings& settings)
	{
		TestContext& context = GetTestContext();
		context.runTests     = 0;
		context.failedTests  = 0;
		context.skippedTests = 0;

		Info("PipeTests: {} describe(s) registered.", context.root.describes.Size());
		TArray<std::function<void()>> beforeHooks;
		TArray<std::function<void()>> afterHooks;
		RunNested(context.root, beforeHooks, afterHooks, settings.filter);

		Info("PipeTests complete: {} run, {} passed, {} failed, {} skipped.", context.runTests,
		    context.runTests - context.failedTests, context.failedTests, context.skippedTests);

		return context.failedTests == 0 ? 0 : 1;
	}

	int RunTests(int argc, char** argv)
	{
		TestSettings settings;
		for (i32 i = 1; i < argc; ++i)
		{
			const StringView arg{argv[i]};
			if (Strings::StartsWith(arg, StringView{"--filter="}))
			{
				settings.filter = Strings::RemoveFromStart(arg, StringView{"--filter="});
			}
			else if (Strings::Equals(arg, StringView{"--filter"}))
			{
				if (i + 1 < argc)
				{
					settings.filter = StringView{argv[++i]};
				}
			}
			else if (Strings::StartsWith(arg, StringView{"--"}))
			{
				Warning("PipeTests: unknown argument '{}'. Ignoring.", arg);
			}
			else if (settings.filter.empty())
			{
				settings.filter = arg;
			}
		}
		return RunTests(settings);
	}
}    // namespace p
