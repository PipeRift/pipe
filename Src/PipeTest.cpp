// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "PipeTest.h"

#include "Pipe/Core/Log.h"
#include "Pipe/Memory/OwnPtr.h"
#include "PipeStrings.h"
#include "PipeTime.h"

#include <cstdio>

#if P_PLATFORM_WINDOWS
	#include <io.h>
	#include <windows.h>
#else
	#include <unistd.h>
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
			TestDescribe root;

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

			bool useColor     = true;
			bool reportTiming = false;

			// Wall time of the last executed test body, in seconds.
			double lastTestDuration = 0.0;
			// Sum of all executed test bodies this run, in seconds.
			double totalDuration = 0.0;
		};

		// Function-local static: initialized on first use regardless of the
		// static-init order of other translation units, so a `P_SPEC` call
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

		// Appends a new describe to `parent` and returns it.
		TestDescribe& AddDescribe(TestDescribe& parent, StringView name)
		{
			TestDescribe describe;
			describe.name = String{name};
			parent.describes.Add(Move(describe));
			return parent.describes.Last();
		}

		// Registers a test in the current describe. `kind` names the API
		// (It/XIt) and is used in error messages.
		void AddTest(StringView kind, StringView name, std::function<void()> fn, bool skip)
		{
			TestDescribe*& current = CurrentDescribe();
			if (!current)
			{
				Error("PipeTest: {}('{}') called outside a Spec. Ignoring.", kind, name);
				return;
			}
			TestCase test;
			test.name = String{name};
			test.body = Move(fn);
			test.skip = skip;
			current->tests.Add(Move(test));
		}
	}    // namespace


	namespace details
	{
		P_API void CountAssert()
		{
			++GetTestContext().currentTestAssertCount;
		}

		P_API void Fail(const std::source_location& loc, StringView message)
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

	P_API void RegisterSpec(StringView name, TFunction<void()> fn)
	{
		TestContext& context    = GetTestContext();
		context.currentDescribe = &AddDescribe(context.root, name);
		fn();
		context.currentDescribe = nullptr;
	}

	P_API void RegisterSpec(TFunction<void()> fn)
	{
		TestContext& context    = GetTestContext();
		context.currentDescribe = &context.root;
		fn();
		context.currentDescribe = nullptr;
	}

	P_API void Describe(StringView name, TFunction<void()> fn)
	{
		TestDescribe*& current = CurrentDescribe();
		if (!current)
		{
			Error("PipeTest: Describe('{}') called outside a Spec. Ignoring.", name);
			return;
		}

		TestDescribe* prevDescribe = current;
		current                    = &AddDescribe(*current, name);
		fn();
		current = prevDescribe;
	}

	P_API void It(StringView name, std::function<void()> fn)
	{
		AddTest("It", name, Move(fn), false);
	}

	P_API void XIt(StringView name, std::function<void()> fn)
	{
		AddTest("XIt", name, Move(fn), true);
	}

	P_API void BeforeEach(std::function<void()> fn)
	{
		TestDescribe*& current = CurrentDescribe();
		if (!current)
		{
			Error("PipeTest: BeforeEach called outside a Spec. Ignoring.");
			return;
		}
		current->beforeEach = fn;
	}

	P_API void AfterEach(std::function<void()> fn)
	{
		TestDescribe*& current = CurrentDescribe();
		if (!current)
		{
			Error("PipeTest: AfterEach called outside a Spec. Ignoring.");
			return;
		}
		current->afterEach = fn;
	}

	P_API TTypeId<ITestReporter> FindReporter(StringView name)
	{
		const TypeId baseId = GetTypeId<ITestReporter>();
		for (const TypeId id : GetRegisteredTypeIds())
		{
			if (!IsTypeParentOf(baseId, id))
			{
				continue;
			}

			// "spec" matches SpecReporter; the full name also matches ("specreporter").
			const StringView typeName = RemoveNamespace(GetTypeName(id));
			if (Strings::IEquals(Strings::RemoveFromEnd(typeName, "Reporter"), name))
			{
				return TTypeId<ITestReporter>{id};
			}
		}
		return TTypeId<ITestReporter>{TypeId{}};    // No match: invalid id
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

		// Shared summary footer (defined below; forward-declared for reporters).
		static void WriteSummary();

		// ---- Spec reporter (default, verbose) ----
		// bandit's `spec` reporter: indented contexts, "- it <name> ... OK".
		struct SpecReporter : ITestReporter
		{
			using Super = ITestReporter;
			P_STRUCT(SpecReporter)

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
			using Super = ITestReporter;
			P_STRUCT(DotsReporter)

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
		// bandit's `singleline` reporter: prints a single self-overwriting
		// progress line on real terminals only. On redirected streams no
		// per-test progress is printed, so the output stays clean; failure
		// details and totals are deferred to TestRunComplete and reported
		// exactly once.
		struct SinglelineReporter : ITestReporter
		{
			using Super = ITestReporter;
			P_STRUCT(SinglelineReporter)

			SinglelineReporter()
			{
#if P_PLATFORM_WINDOWS
				isTty = ::_isatty(::_fileno(stdout));
#else
				isTty = isatty(STDOUT_FILENO) != 0;
#endif
			}

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
				const i32 run        = context.runTests;
				if (run <= 0)
				{
					Error("Could not find any tests.");
					return;
				}
				if (!isTty)
				{
					return;
				}
				DrawLine(StatusLine(context));
			}

			void TestRunComplete() override;

		private:
			// bandit's live status line: only includes the succeeded/failed
			// counts once something has failed, with the failed count red.
			static String StatusLine(const TestContext& context)
			{
				const i32 run = context.runTests;
				if (context.failedTests == 0)
				{
					return Format("Executed {} tests.", run);
				}
				return Format("Executed {} tests. {} succeeded. {}", run, run - context.failedTests,
				    Colored(Red, Format("{} failed.", context.failedTests)));
			}

			// Overwrites the current line in place on a real terminal (console
			// API on Windows, carriage-return elsewhere).
			void DrawLine(StringView text)
			{
#if P_PLATFORM_WINDOWS
				const HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
				if (handle != INVALID_HANDLE_VALUE)
				{
					CONSOLE_SCREEN_BUFFER_INFO info;
					if (GetConsoleScreenBufferInfo(handle, &info) != 0)
					{
						const COORD position = {0, info.dwCursorPosition.Y};
						SetConsoleCursorPosition(handle, position);
						WriteConsoleA(
						    handle, text.data(), static_cast<DWORD>(text.size()), nullptr, nullptr);
						return;
					}
				}
#endif
				std::cout << '\r' << text << std::flush;
			}

			bool isTty = false;
		};

		// ---- Info reporter (verbose with timing support) ----
		// bandit's `info` reporter: "begin/end <context>" lines with per-context
		// totals, "[ PASS ] / [ FAIL ] / -ERROR->" test lines, a failures list
		// and its own summary. Honors --report-timing on every test line.
		struct InfoReporter : ITestReporter
		{
			using Super = ITestReporter;
			P_STRUCT(InfoReporter)

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
			TestContext& context = GetTestContext();

			// Final live line mirrors bandit's singleline reporter: executed/
			// succeeded/failed totals right before the failure summary.
			const String line = StatusLine(context);
			if (isTty)
			{
				DrawLine(line);
				std::cout << std::endl;
			}
			else
			{
				Info("{}", line);
			}
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
				String fullName;
				for (i32 i = 0; i < context.contextStack.Size(); ++i)
				{
					fullName += context.contextStack[i];
					fullName += '.';
				}
				fullName += test.name;
				const bool included = only.empty() || Strings::Contains(fullName, only);
				const bool excluded = !skip.empty() && Strings::Contains(fullName, skip);
				if (test.skip || !included || excluded
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
						context.failures.Add(fullName + ":\nUnknown exception\n");
					}
					else
					{
						reporter.ItFailed(test.name);
						String detail = context.currentFailureDetail;
						context.failures.Add(detail.empty() ? (fullName + ":\n")
						                                    : (fullName + ":\n" + detail + "\n"));
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


	P_API int RunTests(const TestSettings& settings)
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

		const TypeId reporterId = settings.reporter;
		TOwnPtr<ITestReporter> reporter;
		if (reporterId == GetTypeId<DotsReporter>())
		{
			reporter = MakeOwned<DotsReporter>();
		}
		else if (reporterId == GetTypeId<SinglelineReporter>())
		{
			reporter = MakeOwned<SinglelineReporter>();
		}
		else if (reporterId == GetTypeId<InfoReporter>())
		{
			reporter = MakeOwned<InfoReporter>();
		}
		else
		{
			// Spec (also the fallback for an unset/unknown reporter id).
			reporter = MakeOwned<SpecReporter>();
		}

		reporter->TestRunStarting();

		const DateTime runStart = DateTime::Now();

		TArray<std::function<void()>> beforeHooks;
		TArray<std::function<void()>> afterHooks;
		RunNested(context.root, beforeHooks, afterHooks, settings.only, settings.skip,
		    settings.breakOnFailure, *reporter);

		// Total duration is wall time from run start to run end.
		const Timespan runElapsed = DateTime::Now() - runStart;
		context.totalDuration     = runElapsed.GetTotalSeconds<double>();

		reporter->TestRunComplete();

		return context.failedTests == 0 ? 0 : 1;
	}

	P_API int RunTests(int argc, char** argv)
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
			else if (Strings::StartsWith(arg, StringView{"-r="})
			         || Strings::StartsWith(arg, StringView{"--reporter="}))
			{
				StringView name = Strings::RemoveFromStart(arg, StringView{"-r="});
				name            = Strings::RemoveFromStart(name, StringView{"--reporter="});

				settings.reporter = FindReporter(name);
				if (!settings.reporter)
				{
					Warning("PipeTest: unknown reporter '{}'. Using default.", name);
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
