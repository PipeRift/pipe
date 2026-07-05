// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Pipe/Core/Log.h"
#include "Pipe/Core/Subprocess.h"

#include <bandit/bandit.h>
#include <Pipe/Files/PlatformPaths.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


go_bandit([]()
{
	describe("Core.Subprocess", []()
	{
		it("Can run process", [&]()
		{
			AssertThat(p::RunProcess({""}).IsSet(), Equals(false));

#if defined(_MSC_VER)    // Test with a silent command (no stdout)
			AssertThat(p::RunProcess({"cmd", "/c", "exit", "0"}).IsSet(), Equals(true));
#endif
		});
	});
});
