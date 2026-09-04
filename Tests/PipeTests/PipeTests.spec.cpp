// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <PipeStrings.h>

#include <functional>

using namespace p;

static int beforeEachCount = 0;
static int afterEachCount  = 0;
static int topTestResult   = 0;


namespace
{
// Auto-registers via static init (macro-free go_bandit equivalent).
const bool autoRegistered = []()
{
Spec("PipeTests", []()
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
		It("ToEqual / ToNotEqual", []()
		{
			int value = 4;
			Expect(value).ToEqual(4);
			Expect(value).ToNotEqual(5);
		});
		It("Relational", []()
		{
			int value = 4;
			Expect(value).ToBeLess(5);
			Expect(value).ToBeLessOrEqual(4);
			Expect(value).ToBeGreater(3);
			Expect(value).ToBeGreaterOrEqual(4);
		});
		It("Booleans", []()
		{
			bool flag = true;
			Expect(flag).ToBeTrue();
			Expect(!flag).ToBeFalse();
		});
		It("Strings", []()
		{
			Expect("acidic").ToContain("acid");
			Expect(String{"hello"}).ToNotContain("world");
		});
		It("Equals int", []()
		{
			Expect(4).ToEqual(4);
		});
	});
});
return true;
}();
}    // namespace
