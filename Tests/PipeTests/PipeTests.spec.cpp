// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTests.h>

#include <functional>

using namespace p;

static int beforeEachCount = 0;
static int afterEachCount  = 0;
static int topTestResult   = 0;


void RegisterPipeTests()
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
	});
}