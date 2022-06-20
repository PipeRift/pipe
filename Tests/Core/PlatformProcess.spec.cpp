// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/Core/PlatformProcess.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


go_bandit([]() {
	describe("Core.PlatformProcess", []() {
		it("Can create and destroy pipes", [&]() {
			PipeHandle pipe{};
			AssertThat(pipe.IsValid(), Equals(true));

			pipe.Close();
			AssertThat(pipe.IsValid(), Equals(false));
		});
	});
});
