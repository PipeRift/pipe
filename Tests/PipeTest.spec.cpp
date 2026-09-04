// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeStrings.h>
#include <PipeTest.h>

#include <functional>

using namespace p;

static int beforeEachCount = 0;
static int afterEachCount  = 0;
static int topTestResult   = 0;


Spec("PipeTest", []()
{
	BeforeEach([]()
	{
		++beforeEachCount;
	});
	AfterEach([]()
	{
		++afterEachCount;
	});

	Describe("Basics", []()
	{
		It("Registers and runs", []()
		{
			topTestResult = 42;
		});
		XIt("Is skipped", []()
		{
			topTestResult = -1;
		});
	});

	Describe("Expect", []()
	{
		It("Relational", []()
		{
			int value = 4;
			Expect(value).ToBeLess(5);
			Expect(value).ToBeLessOrEqual(4);
			Expect(value).ToBeGreater(3);
			Expect(value).ToBeGreaterOrEqual(4);
		});
		It("Contains", []()
		{
			Expect("acidic").ToContain("acid");
			Expect("acidic").ToNotContain("dictionary");

			Expect(String{"hello"}).ToContain("hello");
			Expect(String{"hello"}).ToContain("llo");
			Expect(String{"hello"}).ToNotContain("world");
			Expect(String{"hello"}).ToNotContain("hello friend");
		});
		It("ToEqual/ToNotEqual", []()
		{
			// Ints
			Expect(4u).ToEqual(4u);
			Expect(4u).ToNotEqual(5u);
			u32 value = 4u;
			Expect(value).ToEqual(4u);
			Expect(value).ToNotEqual(5u);

			Expect(4).ToEqual(4);
			Expect(4).ToNotEqual(5);
			i32 value2 = 4;
			Expect(value2).ToEqual(4);
			Expect(value2).ToNotEqual(5);

			// Bools
			Expect(true).ToBeTrue();
			Expect(false).ToBeFalse();
			bool flag = true;
			Expect(flag).ToBeTrue();
			Expect(!flag).ToBeFalse();
		});
	});
});
