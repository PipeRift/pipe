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
			TestDescribe* currentDescribe = nullptr;
			i32 failedTests               = 0;
			i32 runTests                  = 0;
			i32 skippedTests              = 0;
			i32 currentTestFailureCount   = 0;
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
			Error("PipeTest: {}:{}: {}", loc.file_name(), loc.line(), message);
			++GetTestContext().currentTestFailureCount;
		}
	}    // namespace details


	void RegisterSpec(StringView name, TFunction<void()> fn)
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

	void RegisterSpec(TFunction<void()> fn)
	{
		TestContext& context    = GetTestContext();
		context.currentDescribe = &context.root;
		fn();
		context.currentDescribe = nullptr;
	}

	void Describe(StringView name, TFunction<void()> fn)
	{
		TestDescribe*& current = CurrentDescribe();
		if (!current)
		{
			Error("PipeTest: Describe('{}') called outside a Spec. Ignoring.", name);
			return;
		}

		TestDescribe describe;
		describe.name = String{name};
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
			Error("PipeTest: It('{}') called outside a Spec. Ignoring.", name);
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
			Error("PipeTest: XIt('{}') called outside a Spec. Ignoring.", name);
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
			Error("PipeTest: BeforeEach called outside a Spec. Ignoring.");
			return;
		}
		current->beforeEach = fn;
	}

	void AfterEach(std::function<void()> fn)
	{
		TestDescribe*& current = CurrentDescribe();
		if (!current)
		{
			Error("PipeTest: AfterEach called outside a Spec. Ignoring.");
			return;
		}
		current->afterEach = fn;
	}


	namespace
	{
		// ANSI color codes (used when useColor is true).
		const char* kColorReset  = "\033[0m";
		const char* kColorGreen  = "\033[32m";
		const char* kColorRed    = "\033[31m";
		const char* kColorYellow = "\033[33m";
		const char* kColorCyan   = "\033[36m";
		const char* kColorDim    = "\033[90m";

		static String FullName(const TestDescribe& describe, const TestCase& test)
		{
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

		static void ListNested(const TestDescribe& describe, StringView filter)
		{
			for (const TestDescribe& sub : describe.describes)
			{
				ListNested(sub, filter);
			}
			for (const TestCase& test : describe.tests)
			{
				String full = FullName(describe, test);
				if (MatchesFilter(full, filter))
				{
					if (test.skip)
					{
						Info("  {}[SKIP]{} {}", kColorYellow, kColorReset, full);
					}
					else
					{
						Info("  {}{}{}", kColorDim, full, kColorReset);
					}
				}
			}
		}

		static void RunNested(TestDescribe& describe, TArray<std::function<void()>>& beforeHooks,
		    TArray<std::function<void()>>& afterHooks, StringView filter, bool useColor)
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
				RunNested(sub, beforeHooks, afterHooks, filter, useColor);
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
				bool passed                     = true;
				try
				{
					test.body();
				}
				catch (...)
				{
					passed = false;
					Error("PipeTest: test failed by exception: {}", FullName(describe, test));
				}
				passed = passed && (context.currentTestFailureCount == 0);

				for (i32 i = afterHooks.Size(); i > 0; --i)
				{
					afterHooks[i - 1]();
				}

				String name = FullName(describe, test);
				if (passed)
				{
					if (useColor)
					{
						Info("  {}[PASS]{} {}{}{}{}", kColorGreen, kColorReset, kColorDim, name,
						    kColorReset, kColorReset);
					}
					else
					{
						Info("  [PASS] {}", name);
					}
				}
				else
				{
					++context.failedTests;
					if (useColor)
					{
						Error("  {}[FAIL]{} {}", kColorRed, kColorReset, name);
					}
					else
					{
						Error("  [FAIL] {}", name);
					}
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

		const char* cr = settings.useColor ? kColorReset : "";
		const char* cb = settings.useColor ? kColorCyan : "";

		Info("{}{}describe(s) registered.{}", cb, context.root.describes.Size(), cr);

		// --list: print test names and exit.
		if (settings.listOnly)
		{
			for (TestDescribe& spec : context.root.describes)
			{
				String specName = spec.name.empty() ? String{"(unnamed)"} : String{spec.name};
				Info("{}", specName);
				ListNested(spec, settings.filter);
			}
			return 0;
		}

		TArray<std::function<void()>> beforeHooks;
		TArray<std::function<void()>> afterHooks;
		RunNested(context.root, beforeHooks, afterHooks, settings.filter, settings.useColor);

		i32 passed = context.runTests - context.failedTests;
		if (settings.useColor)
		{
			Info("{}PipeTest{}: {} run, {}{}{} passed{}, {}{}{} failed{}, {}{} skipped{}.", cb,
			    kColorReset, context.runTests, passed > 0 ? kColorGreen : "", passed, cr,
			    context.failedTests > 0 ? kColorRed : "", context.failedTests, cr,
			    context.skippedTests > 0 ? kColorYellow : "", context.skippedTests, cr);
		}
		else
		{
			Info("PipeTest: {} run, {} passed, {} failed, {} skipped.", context.runTests, passed,
			    context.failedTests, context.skippedTests);
		}

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
			else if (Strings::Equals(arg, StringView{"--filter"}) || Strings::Equals(arg, StringView{"-f"}))
			{
				if (i + 1 < argc)
				{
					settings.filter = StringView{argv[++i]};
				}
			}
			else if (Strings::Equals(arg, StringView{"--list"}) || Strings::Equals(arg, StringView{"-l"}))
			{
				settings.listOnly = true;
			}
			else if (Strings::Equals(arg, StringView{"--no-color"}))
			{
				settings.useColor = false;
			}
			else if (Strings::StartsWith(arg, StringView{"--"}))
			{
				Warning("PipeTest: unknown argument '{}'. Ignoring.", arg);
			}
			else if (settings.filter.empty())
			{
				settings.filter = arg;
			}
		}
		return RunTests(settings);
	}
}    // namespace p
