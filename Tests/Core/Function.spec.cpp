// Copyright 2015-2024 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Files/PlatformPaths.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


struct Foo
{
	static bool called;
	static void StaticFunc()
	{
		called = true;
	}
	static void OtherStaticFunc()
	{
		called = true;
	}

	void Func()
	{
		called = true;
	}
};
inline bool Foo::called = false;


go_bandit([]() {
	describe("Core.Function", []() {
		it("Can create empty", [&]() {
			TFunction<void()> func{};
			AssertThat(func.IsBound(), Equals(false));
			AssertThat(bool(func), Equals(false));
		});

		it("Can create from function", [&]() {
			TFunction<void()> func{Foo::StaticFunc};

			AssertThat(func.IsBound(), Equals(true));
		});

		it("Can compare functions", [&]() {
			TFunction<void()> func1{Foo::StaticFunc};
			TFunction<void()> func2{Foo::StaticFunc};
			TFunction<void()> func3{&Foo::StaticFunc};

			TFunction<void()> func4{};

			TFunction<void()> func5{Foo::OtherStaticFunc};

			AssertThat(func1 == func2, Equals(true));
			AssertThat(func1 == func3, Equals(true));
			AssertThat(func1 == func4, Equals(false));
			// AssertThat(func1 == func5, Equals(false));
		});

		it("Can call static functions", [&]() {
			TFunction<void()> func1{Foo::StaticFunc};
			TFunction<void()> func2{&Foo::StaticFunc};

			Foo::called = false;
			func1();
			AssertThat(Foo::called, Equals(true));

			Foo::called = false;
			func2();
			AssertThat(Foo::called, Equals(true));
		});

		it("Can call lambda functions", [&]() {
			static bool called;
			called = false;

			TFunction<void()> func = []() {
				called = true;
			};
			func();
			AssertThat(called, Equals(true));
		});
	});
});
