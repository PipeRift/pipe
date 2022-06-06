// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <PCore/PlatformProcess.h>
#include <PFiles/Paths.h>


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

	void Func() {}
};
inline bool Foo::called = false;


go_bandit([]() {
	describe("Events.Function", []() {
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

		// TODO: Find out why this test fails on some Linux environments
		xit("Can call lambda functions", [&]() {
			bool called = false;

			TFunction<void()> func = [&called]() {
				called = true;
			};
			func();
			AssertThat(called, Equals(true));
		});
	});
});
