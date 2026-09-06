// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Core/Function.h"
#include "Pipe/Core/Macros.h"
#include "Pipe/Core/StringView.h"
#include "PipeReflect.h"
#include "PipeStrings.h"

#include <format>
#include <functional>
#include <memory>
#include <source_location>
#include <string>
#include <type_traits>
#include <utility>


namespace p
{
	/**
	 * Test framework for Pipe and Rift.
	 * Imgui-style global context: registration functions act on a current describe.
	 * Spec opens a first-level describe; Describe/It/BeforeEach/AfterEach attach to
	 * the current describe as functions are called. Spec calls live at file scope
	 * and auto-register via static init (like go_bandit); no macros, no manual
	 * registration calls needed.
	 */

	namespace details
	{
		// Detects whether a type can be rendered via std::format.
		template<typename T, typename Char = char>
		concept FormattableType =
		    requires(const T& value) { std::formatter<std::remove_cvref_t<T>, Char>{}; };
	}    // namespace details

	void RegisterSpec(StringView name, TFunction<void()> fn);
	void RegisterSpec(TFunction<void()> fn);

	// Self-registering top-level describe. Registers its spec body on
	// construction (same pattern as TTypeAutoRegister). The body runs
	// immediately during registration, so TFunction (non-owning) is safe.
	struct SpecAutoRegister
	{
		SpecAutoRegister(StringView name, TFunction<void()> fn)
		{
			RegisterSpec(name, fn);
		}
		SpecAutoRegister(TFunction<void()> fn)
		{
			RegisterSpec(fn);
		}
	};

	// Declares the spec's body function and registers it on static init.
	// The body is written as a plain function block after the macro, so it
	// is NOT part of the macro arguments and preprocessor directives are
	// allowed inside:
	//
	//     P_SPEC("Files.Paths", []()
	//     {
	//         It("Some test", []() {
	//             #if P_PLATFORM_WINDOWS
	//             ...
	//             #endif
	//         });
	//     });
#define P_SPEC static const p::SpecAutoRegister P_CAT(_pipeSpecReg_, __COUNTER__)

	// Nested describe. Only valid inside a Spec; otherwise logs an error and ignores.
	void Describe(StringView name, TFunction<void()> fn);
	// Register a runnable test in the current describe.
	// Bodies are stored until RunTests, so std::function (owning) is required here.
	void It(StringView name, std::function<void()> fn);
	// Register a disabled test; never run.
	void XIt(StringView name, std::function<void()> fn);
	// Setup hook attached to the current describe.
	void BeforeEach(std::function<void()> fn);
	// Teardown hook attached to the current describe.
	void AfterEach(std::function<void()> fn);

	// Reporter interface (defined in Src/Tests/PipeTest.cpp). Forward
	// declaration so TestSettings can reference its type id.
	struct ITestReporter;

	// Settings for a test run.
	struct TestSettings
	{
		StringView only;        // Run only describe/it containing substring.
		StringView skip;        // Skip all describe/it containing substring.
		bool dryRun = false;    // Report full tree as SKIPPED, run nothing (bandit semantics).
		bool breakOnFailure = false;    // Stop the test run on the first failing test.
		bool useColor       = true;     // Colorized output.
		bool reportTiming   = false;    // Report per-test timing information.
		// Reporter to use, identified by type. Invalid (default) means Spec.
		// Bound to ITestReporter, so only reporter types can be assigned
		// (e.g. TTypeId<ITestReporter>::Of<DotsReporter>()).
		TTypeId<ITestReporter> reporter;
	};

	int RunTests(const TestSettings& settings);
	int RunTests(int argc, char** argv);


	// Extensible value-to-string hook for failure messages.
	// Specialize for user types. Default handles numbers and string views.
	template<typename T>
	inline String TestString(const T& value);

	template<typename T>
	inline String TestString(const T& value)
	{
		if constexpr (details::FormattableType<T>)
		{
			return Format("{}", value);
		}
		else
		{
			// Non-formattable type (structs, containers, byte views...).
			// Render a generic placeholder instead of failing to compile.
			(void)value;
			return Format("<value@{}>", static_cast<const void*>(std::addressof(value)));
		}
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

	inline String TestString(const String& value)
	{
		return String{value};
	}

	inline String TestString(const char* value)
	{
		return value ? String{value} : String{"(null)"};
	}

	inline String TestString(const std::string& value)
	{
		return String{StringView{value}};
	}

	// Any other pointer is shown as its address so failure messages stay formattable.
	template<typename T>
	inline String TestString(const T* value)
	{
		return Format("{}", static_cast<const void*>(value));
	}

	namespace details
	{
		// Format failure message from source location + description.
		void Fail(const std::source_location& loc, StringView message);

		// Counts an assertion. Used to detect tests that ran no expects.
		void CountAssert();

		// Detect types constructible into a StringView (string-ish values).
		template<typename T>
		concept IsStringLike = requires(const T& value) { StringView{value}; };

		// Compares two possibly-different types: string-ish values compare by view,
		// everything else uses operator==.
		template<typename A, typename E>
		bool ValuesEqual(const A& a, const E& e)
		{
			if constexpr (IsStringLike<A> && IsStringLike<E>)
			{
				return StringView{a} == StringView{e};
			}
			else
			{
				return a == e;
			}
		}
	}    // namespace details


	// ---- fluent assertion ----
	template<typename Actual>
	class ExpectValue
	{
	public:
		ExpectValue(const Actual& value, const std::source_location& loc) : value(value), loc(loc)
		{
			details::CountAssert();
		}

		template<typename Expected>
		void ToEqual(const Expected& expected) const
		{
			if (!details::ValuesEqual(value, expected))
			{
				details::Fail(loc,
				    Format("Expected {} to equal {}", TestString(value), TestString(expected)));
			}
		}

		template<typename Expected>
		void ToNotEqual(const Expected& expected) const
		{
			if (details::ValuesEqual(value, expected))
			{
				details::Fail(loc,
				    Format("Expected {} to not equal {}", TestString(value), TestString(expected)));
			}
		}

		void ToBeLess(const Actual& other) const
		{
			if (!(value < other))
			{
				details::Fail(loc,
				    Format("Expected {} to be less than {}", TestString(value), TestString(other)));
			}
		}

		void ToBeLessOrEqual(const Actual& other) const
		{
			if (!(value <= other))
			{
				details::Fail(loc, Format("Expected {} to be less or equal to {}",
				                       TestString(value), TestString(other)));
			}
		}

		void ToBeGreater(const Actual& other) const
		{
			if (!(value > other))
			{
				details::Fail(loc, Format("Expected {} to be greater than {}", TestString(value),
				                       TestString(other)));
			}
		}

		void ToBeGreaterOrEqual(const Actual& other) const
		{
			if (!(value >= other))
			{
				details::Fail(loc, Format("Expected {} to be greater or equal to {}",
				                       TestString(value), TestString(other)));
			}
		}

		void ToBeTrue() const
		{
			if (!value)
			{
				details::Fail(loc, "Expected value to be true");
			}
		}

		void ToBeFalse() const
		{
			if (value)
			{
				details::Fail(loc, "Expected value to be false");
			}
		}

		void ToContain(const StringView sub) const
		{
			StringView view{value};
			if (Strings::Find(view, sub) == StringView::npos)
			{
				details::Fail(
				    loc, Format("Expected {} to contain {}", TestString(value), TestString(sub)));
			}
		}

		void ToNotContain(const StringView sub) const
		{
			StringView view{value};
			if (Strings::Find(view, sub) != StringView::npos)
			{
				details::Fail(loc,
				    Format("Expected {} to not contain {}", TestString(value), TestString(sub)));
			}
		}

	private:
		const Actual& value;
		std::source_location loc;
	};

	// Returns a matcher bound to the caller's source location for reporting.
	template<typename T>
	ExpectValue<T> Expect(
	    const T& value, const std::source_location loc = std::source_location::current())
	{
		return ExpectValue<T>(value, loc);
	}
};    // namespace p
