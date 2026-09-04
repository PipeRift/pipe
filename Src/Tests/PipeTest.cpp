// Copyright 2015-2026 Piperift. All Rights Reserved.

#ifndef P_OVERRIDE_NEWDELETE
	#define P_OVERRIDE_NEWDELETE 1
#endif
#if P_OVERRIDE_NEWDELETE
	#include "PipeNewDelete.h"
#endif

#include "Pipe.h"
#include "Pipe/Core/Log.h"
#include "Pipe/Memory/UniquePtr.h"
#include "PipeStrings.h"
#include "PipeTest.h"
#include "PipeTime.h"

#if P_PLATFORM_WINDOWS
	#include <windows.h>
#endif


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
			// Assertions made by the current test (0 = no expects ran).
			i32 currentTestAssertCount = 0;
			// Set once a test has failed (--break-on-failure stops the run).
			bool encounteredFailure = false;

			// Active describe names during a run (outermost first).
			TArray<String> contextStack;
			// Formatted failure blocks, one per failed test: "context it:\nmessage\n".
			TArray<String> failures;
			// Assertion detail accumulated for the current test (file:line: msg).
			String currentFailureDetail;

			TestReporter reporter = TestReporter::Spec;
			bool useColor         = true;
			bool reportTiming     = false;

			// Wall time of the last executed test body, in seconds.
			double lastTestDuration = 0.0;
			// Sum of all executed test bodies this run, in seconds.
			double totalDuration = 0.0;
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
		void CountAssert()
		{
			++GetTestContext().currentTestAssertCount;
		}

		void Fail(const std::source_location& loc, StringView message)
		{
			// Not printed immediately: failures are reported by the reporter
			// at the end of the run, so they never interleave with deferred
			// context output.
			TestContext& context = GetTestContext();
			++context.currentTestFailureCount;
			if (!context.currentFailureDetail.empty())
			{
				context.currentFailureDetail += "\n";
			}
			context.currentFailureDetail +=
			    Format("{}:{}: {}", loc.file_name(), loc.line(), message);
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
		// Color aliases for readability.
		using Terminal::Blue;
		using Terminal::BrightBlack;
		using Terminal::ColorReset;
		using Terminal::Cyan;
		using Terminal::Green;
		using Terminal::Red;
		using Terminal::Yellow;

		// Full test name: all enclosing describe names plus the test name,
		// e.g. "Containers.BitArray.Copy.Can copy empty". Used for filtering
		// and failure reports, so `--only` matches any parent describe too.
		static String FullName(StringView testName)
		{
			TestContext& context = GetTestContext();
			String result;
			for (i32 i = 0; i < context.contextStack.Size(); ++i)
			{
				result += context.contextStack[i];
				result += '.';
			}
			result += testName;
			return result;
		}

		// Whether a test (by full describe+it name) should run given the
		// `only`/`skip` substring selection and the skip set.
		static bool Matches(StringView fullName, StringView only, StringView skip)
		{
			const bool included = only.empty() || Strings::Contains(fullName, only);
			const bool excluded = !skip.empty() && Strings::Contains(fullName, skip);
			return included && !excluded;
		}

		// Color a string for terminal output, honoring the useColor flag.
		static String Colored(const char* color, StringView text)
		{
			TestContext& context = GetTestContext();
			if (!context.useColor)
			{
				return String{text};
			}
			return Format("{}{}{}", color, text, Terminal::ColorReset);
		}

		// Formats a duration with 2 significant digits, switching to smaller
		// units before std-format would fall back to exponent notation:
		// 0.0001208 -> "0.00012s", 0.000012 -> "0.012ms", 4e-07 -> "0.4us".
		static String FormatDuration(double seconds)
		{
			if (seconds == 0)
			{
				return "0s";
			}
			const double abs = seconds < 0 ? -seconds : seconds;
			if (abs >= 100)
			{
				return Format("{:.0f}s", seconds);
			}
			if (abs >= 1e-4)
			{
				return Format("{:.2}s", seconds);
			}
			if (abs >= 1e-7)
			{
				return Format("{:.2}ms", seconds * 1e3);
			}
			if (abs >= 1e-10)
			{
				return Format("{:.2}us", seconds * 1e6);
			}
			return Format("{:.2}ns", seconds * 1e9);
		}

		// Timing suffix for test status lines, e.g. " (0.00012s)".
		// Empty unless --report-timing was set.
		static String TimingSuffix()
		{
			TestContext& context = GetTestContext();
			if (!context.reportTiming)
			{
				return {};
			}
			return Colored(Yellow, Format(" ({})", FormatDuration(context.lastTestDuration)));
		}

		// ---- Reporter interface ----
		// Mirrors bandit's reporter callbacks. Each reporter formats the run
		// differently; all share the same per-test execution flow in RunNested.
		struct ITestReporter
		{
			virtual ~ITestReporter() = default;

			virtual void TestRunStarting() {}
			virtual void TestRunComplete() = 0;
			virtual void ContextStarting(StringView) {}
			virtual void ContextEnded(StringView) {}
			virtual void ItStarting(StringView) {}
			virtual void ItSucceeded(StringView) {}
			// Test passed but made no assertions (e.g. smoke tests).
			virtual void ItSucceededNoAssertions(StringView) {}
			virtual void ItFailed(StringView) {}
			virtual void ItUnknownError(StringView) {}
			virtual void ItSkipped(StringView) {}
		};

		// Shared summary footer (defined below; forward-declared for reporters).
		static void WriteSummary();

		// ---- Spec reporter (default, verbose) ----
		// bandit's `spec` reporter: indented contexts, "- it <name> ... OK".
		struct SpecReporter : ITestReporter
		{
			i32 indentation = 0;
			String lastIt;

			String Indent() const
			{
				// One tab per level: the bandit VSCode adapter's parser pops
				// one parent per character of indentation decrease, so any
				// multi-space indent breaks its hierarchy detection.
				String result;
				result.assign(indentation, '\t');
				return result;
			}


			void ContextStarting(StringView desc) override
			{
				Info("{}describe {}", Indent(), desc);
				++indentation;
			}

			void ContextEnded(StringView) override
			{
				--indentation;
			}

			void ItStarting(StringView desc) override
			{
				lastIt = String{desc};
			}

			void ItSucceeded(StringView) override
			{
				Info("{}- it {} ... {}{}", Indent(), lastIt, Colored(Green, "OK"), TimingSuffix());
			}

			void ItSucceededNoAssertions(StringView) override
			{
				Info("{}- it {} ... {}{}", Indent(), lastIt, Colored(Yellow, "OK"), TimingSuffix());
			}

			void ItFailed(StringView) override
			{
				// stdout, not stderr: tooling (e.g. the bandit VSCode adapter)
				// parses test results from stdout only.
				Info(
				    "{}- it {} ... {}{}", Indent(), lastIt, Colored(Red, "FAILED"), TimingSuffix());
			}

			void ItUnknownError(StringView) override
			{
				Info("{}- it {} ... {}{}", Indent(), lastIt, Colored(Red, "ERROR"), TimingSuffix());
			}

			void ItSkipped(StringView desc) override
			{
				Info("{}- it {} ... {}", Indent(), desc, Colored(Yellow, "SKIPPED"));
			}

			void TestRunComplete() override;
		};

		// ---- Dots reporter (compact) ----
		// bandit's `dots` reporter: one character per test, laid out on shared
		// lines (a fresh line every kLineWidth tests) rather than one line each.
		struct DotsReporter : ITestReporter
		{
			bool anyResults = false;

			void ItSucceeded(StringView) override
			{
				std::cout << Colored(Green, ".");
				anyResults = true;
			}

			void ItSucceededNoAssertions(StringView) override
			{
				std::cout << Colored(Yellow, ".");
				anyResults = true;
			}

			void ItFailed(StringView) override
			{
				std::cout << Colored(Red, "F");
				anyResults = true;
			}

			void ItUnknownError(StringView) override
			{
				std::cout << Colored(Red, "E");
				anyResults = true;
			}

			void ItSkipped(StringView) override
			{
				std::cout << Colored(Yellow, "S");
				anyResults = true;
			}

			void TestRunComplete() override
			{
				if (anyResults)
				{
					std::cout << std::endl;
				}
				WriteSummary();
			}
		};

		// ---- Singleline reporter ----
		// bandit's `singleline` reporter: prints a live status line after each test.
		struct SinglelineReporter : ITestReporter
		{
			void ItSucceeded(StringView) override
			{
				PrintStatus();
			}

			void ItSucceededNoAssertions(StringView) override
			{
				PrintStatus();
			}

			void ItFailed(StringView) override
			{
				PrintStatus();
			}

			void ItUnknownError(StringView) override
			{
				PrintStatus();
			}

			void PrintStatus()
			{
				TestContext& context = GetTestContext();
				i32 run              = context.runTests;
				i32 failed           = context.failedTests;
				i32 passed           = run - failed;
				if (run <= 0)
				{
					Error("Could not find any tests.");
					return;
				}

				Info("Executed {} tests.", run);
				if (failed == 0)
				{
					if (failed <= 0)
					{}
					Info("{}\n {} failed.", run, passed, Colored(Red, Format("{}", failed)));
				}
				else
				{
					Info("Executed {} tests.", run);
				}
			}

			void TestRunComplete() override;
		};

		// ---- Info reporter (verbose with timing support) ----
		// bandit's `info` reporter: "begin/end <context>" lines with per-context
		// totals, "[ PASS ] / [ FAIL ] / -ERROR->" test lines, a failures list
		// and its own summary. Honors --report-timing on every test line.
		struct InfoReporter : ITestReporter
		{
			// One entry per active describe, outermost first.
			struct ContextInfo
			{
				String name;
				i32 total   = 0;
				i32 skipped = 0;
				i32 failed  = 0;
			};

			TArray<ContextInfo> stack;
			// Depth of contexts whose begin/end lines are visible.
			i32 activeIndex = 0;

			String Indent() const
			{
				String result;
				result.assign(size_t(activeIndex) * 2, ' ');
				return result;
			}

			void TestRunStarting() override {}

			void ContextStarting(StringView desc) override
			{
				Info("{}{} {}", Indent(), Colored(Blue, "begin"), desc);
				ContextInfo info;
				info.name = String{desc};
				stack.Add(info);
				++activeIndex;
			}

			void ContextEnded(StringView) override
			{
				ContextInfo& info = stack.Last();
				--activeIndex;
				String line = Format("{}{} {}", Indent(), Colored(Blue, "end"), info.name);
				if (info.total > 0)
				{
					line += Format(" {} total", info.total);
				}
				if (info.skipped > 0)
				{
					line += Colored(Yellow, Format(" {} skipped", info.skipped));
				}
				if (info.failed > 0)
				{
					line += Colored(Red, Format(" {} failed", info.failed));
				}
				Info("{}", line);

				// Merge counts into the parent context.
				if (stack.Size() > 1)
				{
					ContextInfo& parent = stack[stack.Size() - 2];
					parent.total += info.total;
					parent.skipped += info.skipped;
					parent.failed += info.failed;
				}
				stack.RemoveLast();
			}

			void ItStarting(StringView) override {}

			void ItSucceeded(StringView desc) override
			{
				stack.Last().total++;
				Info("{}{} it {}{}", Indent(), Colored(Green, "[ PASS ]"), desc, TimingSuffix());
			}

			// No assertions ran (smoke test): show as a neutral [ TEST ].
			void ItSucceededNoAssertions(StringView desc) override
			{
				stack.Last().total++;
				Info("{}{} it {}{}", Indent(), Colored(Yellow, "[ TEST ]"), desc, TimingSuffix());
			}

			void ItFailed(StringView desc) override
			{
				ContextInfo& info = stack.Last();
				info.total++;
				info.failed++;
				Info("{}{} it {}{}", Indent(), Colored(Red, "[ FAIL ]"), desc, TimingSuffix());
			}

			void ItUnknownError(StringView desc) override
			{
				ContextInfo& info = stack.Last();
				info.total++;
				info.failed++;
				Info("{}{} it {}{}", Indent(), Colored(Red, "-ERROR->"), desc, TimingSuffix());
			}

			// bandit's info reporter counts skipped tests silently.
			void ItSkipped(StringView) override
			{
				ContextInfo& info = stack.Last();
				info.total++;
				info.skipped++;
			}

			void TestRunComplete() override
			{
				TestContext& context = GetTestContext();
				i32 succeeded        = context.runTests - context.failedTests;

				Info("");
				if (context.failedTests > 0)
				{
					Info("{}", Colored(Red, "List of failures:"));
					for (const String& failure : context.failures)
					{
						Info(" (*) {}", Colored(Red, failure));
					}
				}

				Info("Tests run: {}", context.runTests);
				if (context.skippedTests > 0)
				{
					Info("{}", Colored(Yellow, Format("Skipped: {}", context.skippedTests)));
				}
				if (succeeded > 0)
				{
					Info("{}", Colored(Green, Format("Passed: {}", succeeded)));
				}
				if (context.failedTests > 0)
				{
					Info("{}", Colored(Red, Format("Failed: {}", context.failedTests)));
				}
				if (context.reportTiming)
				{
					Info("{}", Colored(Yellow, Format("Total time: {}",
					                               FormatDuration(context.totalDuration))));
				}
			}
		};

		// ---- Summary (shared by all reporters) ----
		// bandit's summary: success/failure header + failure blocks + totals line.
		static void WriteSummary()
		{
			TestContext& context = GetTestContext();
			i32 succeeded        = context.runTests - context.failedTests;

			if (context.failedTests == 0)
			{
				Info("{}", Colored(Green, "Success!"));
			}
			else
			{
				// stdout, not stderr: tooling parses results from stdout only.
				Info("{}", Colored(Red, "There were failures!"));
				for (const String& failure : context.failures)
				{
					Info("{}", failure);
				}
			}

			String line = Format(
			    "Test run complete. {} tests run. {} succeeded.", context.runTests, succeeded);
			if (context.skippedTests > 0)
			{
				line += Format(" {} skipped.", context.skippedTests);
			}
			if (context.failedTests > 0)
			{
				line += Format(" {} failed.", context.failedTests);
			}
			Info("{}", line);
			if (context.reportTiming)
			{
				Info("{}", Colored(Yellow,
				               Format("Total time: {}", FormatDuration(context.totalDuration))));
			}
		}

		void SpecReporter::TestRunComplete()
		{
			// A newline separates per-test output from the summary block.
			Info("");
			WriteSummary();
		}

		void SinglelineReporter::TestRunComplete()
		{
			WriteSummary();
		}

		static void RunNested(TestDescribe& describe, TArray<std::function<void()>>& beforeHooks,
		    TArray<std::function<void()>>& afterHooks, StringView only, StringView skip,
		    bool breakOnFailure, ITestReporter& reporter)
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

			if (!describe.name.empty())
			{
				context.contextStack.Add(describe.name);
				reporter.ContextStarting(describe.name);
			}

			for (TestDescribe& sub : describe.describes)
			{
				RunNested(sub, beforeHooks, afterHooks, only, skip, breakOnFailure, reporter);
			}

			for (TestCase& test : describe.tests)
			{
				// Bandit semantics: tests not selected by the filters, and
				// tests marked skip, are reported as SKIPPED, not hidden.
				// With break-on-failure, everything after the first failure
				// is skipped too.
				if (test.skip || !Matches(FullName(test.name), only, skip)
				    || (breakOnFailure && context.encounteredFailure))
				{
					++context.skippedTests;
					reporter.ItSkipped(test.name);
					continue;
				}
				++context.runTests;
				reporter.ItStarting(test.name);

				for (auto& hook : beforeHooks)
				{
					hook();
				}

				context.currentTestFailureCount = 0;
				context.currentTestAssertCount  = 0;
				context.currentFailureDetail    = {};
				bool passed                     = true;
				bool unknown                    = false;
				const DateTime testStart        = DateTime::Now();
				try
				{
					test.body();
				}
				catch (...)
				{
					passed  = false;
					unknown = true;
				}
				passed                     = passed && (context.currentTestFailureCount == 0);
				const Timespan testElapsed = DateTime::Now() - testStart;
				context.lastTestDuration   = testElapsed.GetTotalSeconds<double>();

				for (i32 i = afterHooks.Size(); i > 0; --i)
				{
					afterHooks[i - 1]();
				}

				String full = FullName(test.name);

				if (passed)
				{
					if (context.currentTestAssertCount == 0)
					{
						reporter.ItSucceededNoAssertions(test.name);
					}
					else
					{
						reporter.ItSucceeded(test.name);
					}
				}
				else
				{
					++context.failedTests;
					context.encounteredFailure = true;
					if (unknown)
					{
						reporter.ItUnknownError(test.name);
						context.failures.Add(full + ":\nUnknown exception\n");
					}
					else
					{
						reporter.ItFailed(test.name);
						String detail = context.currentFailureDetail;
						context.failures.Add(
						    detail.empty() ? (full + ":\n") : (full + ":\n" + detail + "\n"));
					}
				}
			}

			if (!describe.name.empty())
			{
				reporter.ContextEnded(describe.name);
				context.contextStack.RemoveLast();
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

		// Dry run: report the full test tree without executing anything.
		// Matches bandit's run policy, where `--dry-run` skips every `it`
		// (reported as SKIPPED) before any `--only`/`--skip` filter applies,
		// so the whole structure is always visible. Test tooling relies on
		// this to discover tests.
		static void DryRunNested(TestDescribe& describe, ITestReporter& reporter)
		{
			TestContext& context = GetTestContext();
			if (!describe.name.empty())
			{
				context.contextStack.Add(describe.name);
				reporter.ContextStarting(describe.name);
			}

			for (TestDescribe& sub : describe.describes)
			{
				DryRunNested(sub, reporter);
			}

			for (TestCase& test : describe.tests)
			{
				reporter.ItSkipped(test.name);
			}

			if (!describe.name.empty())
			{
				reporter.ContextEnded(describe.name);
				context.contextStack.RemoveLast();
			}
		}
	}    // namespace


	int RunTests(const TestSettings& settings)
	{
#if P_PLATFORM_WINDOWS
		// Enable ANSI escape sequences on the Windows console, otherwise
		// color codes print as garbage (e.g. "←[32m"). The flag is per
		// handle, so both stdout (reporters) and stderr (Error/Warning logs)
		// need it.
		{
			const DWORD handleIds[] = {STD_OUTPUT_HANDLE, STD_ERROR_HANDLE};
			for (DWORD id : handleIds)
			{
				HANDLE handle = GetStdHandle(id);
				DWORD mode    = 0;
				if (handle != INVALID_HANDLE_VALUE && GetConsoleMode(handle, &mode))
				{
					SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
				}
			}
		}
#endif
		TestContext& context = GetTestContext();
		context.runTests     = 0;
		context.failedTests  = 0;
		context.skippedTests = 0;
		context.reporter     = settings.reporter;
		context.useColor     = settings.useColor;
		context.reportTiming = settings.reportTiming;
		context.contextStack.Clear();
		context.failures.Clear();
		context.encounteredFailure = false;

		// --dry-run/--list: report the full tree without executing (bandit
		// semantics). Always uses the spec reporter; it is the only format
		// with a meaningful test listing.
		if (settings.dryRun)
		{
			SpecReporter specReporter;
			specReporter.TestRunStarting();
			DryRunNested(context.root, specReporter);
			specReporter.TestRunComplete();
			return 0;
		}

		TUniquePtr<ITestReporter> reporter;
		switch (settings.reporter)
		{
			case TestReporter::Dots: reporter = MakeUnique<DotsReporter>(); break;
			case TestReporter::Singleline: reporter = MakeUnique<SinglelineReporter>(); break;
			case TestReporter::Info: reporter = MakeUnique<InfoReporter>(); break;
			case TestReporter::Spec:
			default: reporter = MakeUnique<SpecReporter>(); break;
		}

		reporter->TestRunStarting();

		const DateTime runStart = DateTime::Now();

		TArray<std::function<void()>> beforeHooks;
		TArray<std::function<void()>> afterHooks;
		RunNested(context.root, beforeHooks, afterHooks, settings.only, settings.skip,
		    settings.breakOnFailure, *reporter.Get());

		// Total duration is wall time from run start to run end.
		const Timespan runElapsed = DateTime::Now() - runStart;
		context.totalDuration     = runElapsed.GetTotalSeconds<double>();

		reporter->TestRunComplete();

		return context.failedTests == 0 ? 0 : 1;
	}

	int RunTests(int argc, char** argv)
	{
		TestSettings settings;
		for (i32 i = 1; i < argc; ++i)
		{
			const StringView arg{argv[i]};
			if (Strings::StartsWith(arg, StringView{"--only="}))
			{
				settings.only = Strings::RemoveFromStart(arg, StringView{"--only="});
			}
			else if (Strings::StartsWith(arg, StringView{"--skip="}))
			{
				settings.skip = Strings::RemoveFromStart(arg, StringView{"--skip="});
			}
			else if (Strings::StartsWith(arg, StringView{"--reporter="}))
			{
				const StringView name = Strings::RemoveFromStart(arg, StringView{"--reporter="});
				if (Strings::Equals(name, StringView{"dots"}))
				{
					settings.reporter = TestReporter::Dots;
				}
				else if (Strings::Equals(name, StringView{"singleline"}))
				{
					settings.reporter = TestReporter::Singleline;
				}
				else if (Strings::Equals(name, StringView{"spec"}))
				{
					settings.reporter = TestReporter::Spec;
				}
				else if (Strings::Equals(name, StringView{"info"}))
				{
					settings.reporter = TestReporter::Info;
				}
				else
				{
					Warning("PipeTest: unknown reporter '{}'. Using 'dots'.", name);
				}
			}
			else if (Strings::Equals(arg, StringView{"--reporter"})
			         || Strings::Equals(arg, StringView{"-r"}))
			{
				if (i + 1 < argc)
				{
					const StringView name{argv[++i]};
					if (Strings::Equals(name, StringView{"dots"}))
					{
						settings.reporter = TestReporter::Dots;
					}
					else if (Strings::Equals(name, StringView{"singleline"}))
					{
						settings.reporter = TestReporter::Singleline;
					}
					else if (Strings::Equals(name, StringView{"spec"}))
					{
						settings.reporter = TestReporter::Spec;
					}
					else if (Strings::Equals(name, StringView{"info"}))
					{
						settings.reporter = TestReporter::Info;
					}
					else
					{
						Warning("PipeTest: unknown reporter '{}'. Using 'dots'.", name);
					}
				}
			}
			else if (Strings::Equals(arg, StringView{"--report-timing"}))
			{
				settings.reportTiming = true;
			}
			else if (Strings::StartsWith(arg, StringView{"--colorizer="}))
			{
				const StringView name = Strings::RemoveFromStart(arg, StringView{"--colorizer="});
				if (Strings::Equals(name, StringView{"off"}))
				{
					settings.useColor = false;
				}
				// 'dark'/'light' keep color enabled (default).
			}
			else if (Strings::Equals(arg, StringView{"--version"}))
			{
				Info("Pipe version {}", P_VERSION);
				return 0;
			}
			else if (Strings::Equals(arg, StringView{"--help"}))
			{
				Info("USAGE: <executable> [options]");
				Info("");
				Info("Options:");
				Info("  --version,               Print version of Pipe");
				Info("  --help,                  Print usage and exit.");
				Info(
				    "  --skip=<substring>,      Skip all 'describe' and 'it' containing substring");
				Info(
				    "  --only=<substring>,      Run only 'describe' and 'it' containing substring");
				Info("  --break-on-failure,      Stop test run on first failing test");
				Info("  --dry-run,               Skip all tests. Use to list available tests");
				Info("  --report-timing,         Instruct reporter to report timing information");
				Info("  --reporter=<reporter>,   Select reporter: dots, info, singleline, spec");
				Info("  --colorizer=<colorizer>, Select color theme: off");
				Info("  --no-color,              Disable colorized output");
				return 0;
			}
			else if (Strings::Equals(arg, StringView{"--break-on-failure"}))
			{
				settings.breakOnFailure = true;
			}
			else if (Strings::Equals(arg, StringView{"--list"})
			         || Strings::Equals(arg, StringView{"-l"})
			         || Strings::Equals(arg, StringView{"--dry-run"}))
			{
				settings.dryRun = true;
			}
			else if (Strings::Equals(arg, StringView{"--no-color"})
			         || Strings::Equals(arg, StringView{"-c"}))
			{
				settings.useColor = false;
			}
			else if (Strings::StartsWith(arg, StringView{"--"}))
			{
				// Warning("PipeTest: unknown argument '{}'. Ignoring.", arg);
			}
		}
		return RunTests(settings);
	}
}    // namespace p
