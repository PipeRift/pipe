// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Files/PlatformPaths.h>


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


namespace
{
// Auto-registers via static init (macro-free go_bandit equivalent).
const bool autoRegistered = []()
{
Spec("Core.Function", []()
{
	It("Can create empty", []()
	{
		TFunction<void()> func{};
		Expect(func.IsBound()).ToEqual(false);
		Expect(bool(func)).ToEqual(false);
	});

	It("Can create from function", []()
	{
		TFunction<void()> func{Foo::StaticFunc};

		Expect(func.IsBound()).ToEqual(true);
	});

	It("Can compare functions", []()
	{
		TFunction<void()> func1{Foo::StaticFunc};
		TFunction<void()> func2{Foo::StaticFunc};
		TFunction<void()> func3{&Foo::StaticFunc};

		TFunction<void()> func4{};

		TFunction<void()> func5{Foo::OtherStaticFunc};

		Expect(func1 == func2).ToEqual(true);
		Expect(func1 == func3).ToEqual(true);
		Expect(func1 == func4).ToEqual(false);
		// Expect(func1 == func5).ToEqual(false);
	});

	It("Can call static functions", []()
	{
		TFunction<void()> func1{Foo::StaticFunc};
		TFunction<void()> func2{&Foo::StaticFunc};

		Foo::called = false;
		func1();
		Expect(Foo::called).ToEqual(true);

		Foo::called = false;
		func2();
		Expect(Foo::called).ToEqual(true);
	});

	It("Can call lambda functions", []()
	{
		static bool called;
		called = false;

		TFunction<void()> func = []()
		{
			called = true;
		};
		func();
		Expect(called).ToEqual(true);
	});
});
return true;
}();
}    // namespace
