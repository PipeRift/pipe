// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Core/StringView.h"
#include "PipeStrings.h"

#include <functional>
#include <source_location>


namespace p
{
	/**
	 * Test framework for Pipe and Rift.
	 * Imgui-style global context: registration functions act on a current group.
	 * Spec opens a first-level group; Describe/It/BeforeEach/AfterEach attach to
	 * the current group as functions are called. Registration runs inside a
	 * function (e.g. a Register*Tests() routine called from main) — the framework
	 * uses no macros, so specs must not be registered at namespace scope.
	 */

	// Self-registering top-level. Spec(name, fn) opens a first group named `name`.
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


	// Extensible value-to-string hook for failure messages.
	// Specialize for user types. Default handles numbers and string views.
	template<typename T>
	inline String TestString(const T& value);

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

	inline String TestString(const String& value)
	{
		return String{value};
	}

	inline String TestString(const char* value)
	{
		return value ? String{value} : String{"(null)"};
	}

	namespace details
	{
		// Format failure message from source location + description.
		void Fail(const std::source_location& loc, StringView message);
	}    // namespace details


	// ---- fluent assertion ----
	template<typename Actual>
	class ExpectValue
	{
	public:
		ExpectValue(const Actual& value, const std::source_location& loc)
		    : value(value)
		    , loc(loc)
		{}

		void ToEqual(const Actual& expected) const
		{
			if (!(value == expected))
			{
				details::Fail(loc, Format(
				    "Expected {} to equal {}", TestString(value), TestString(expected)));
			}
		}

		void ToNotEqual(const Actual& expected) const
		{
			if (!(value != expected))
			{
				details::Fail(loc, Format(
				    "Expected {} to not equal {}", TestString(value), TestString(expected)));
			}
		}

		void ToBeLess(const Actual& other) const
		{
			if (!(value < other))
			{
				details::Fail(loc, Format(
				    "Expected {} to be less than {}", TestString(value), TestString(other)));
			}
		}

		void ToBeLessOrEqual(const Actual& other) const
		{
			if (!(value <= other))
			{
				details::Fail(loc, Format(
				    "Expected {} to be less or equal to {}", TestString(value), TestString(other)));
			}
		}

		void ToBeGreater(const Actual& other) const
		{
			if (!(value > other))
			{
				details::Fail(loc, Format(
				    "Expected {} to be greater than {}", TestString(value), TestString(other)));
			}
		}

		void ToBeGreaterOrEqual(const Actual& other) const
		{
			if (!(value >= other))
			{
				details::Fail(loc, Format(
				    "Expected {} to be greater or equal to {}", TestString(value), TestString(other)));
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
				details::Fail(loc, Format(
				    "Expected {} to contain {}", TestString(value), TestString(sub)));
			}
		}

		void ToNotContain(const StringView sub) const
		{
			StringView view{value};
			if (Strings::Find(view, sub) != StringView::npos)
			{
				details::Fail(loc, Format(
				    "Expected {} to not contain {}", TestString(value), TestString(sub)));
			}
		}

	private:
		const Actual& value;
		std::source_location loc;
	};

	// Returns a matcher bound to the caller's source location for reporting.
	template<typename T>
	ExpectValue<T> Expect(const T& value, const std::source_location loc = std::source_location::current())
	{
		return ExpectValue<T>(value, loc);
	}
};    // namespace p
