// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Core/Log.h"
#include "Pipe/Core/Subprocess.h"

#include <bandit/bandit.h>
#include <Pipe/Core/PlatformProcess.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


go_bandit([]() {
	describe("Core.Subprocess", []() {
		it("Can run process", [&]() {
			AssertThat(p::RunProcess({""}).IsSet(), Equals(false));
			AssertThat(p::RunProcess({"whoami"}).IsSet(), Equals(true));
		});
	});
});
