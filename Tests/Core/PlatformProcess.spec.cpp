// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Core/Generic/GenericPlatformProcess.h"
#include "Pipe/Core/Windows/WindowsPlatformProcess.h"

#include <bandit/bandit.h>
#include <Pipe/Core/PlatformProcess.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


go_bandit([]() {
	describe("Core.PlatformProcess", []() {
		it("Can create and destroy pipes", [&]() {
			PipeHandle pipe{};
			AssertThat(pipe.readPipe, !Equals(nullptr));
			AssertThat(pipe.writePipe, !Equals(nullptr));

			pipe.Close();
			AssertThat(pipe.readPipe, Equals(nullptr));
			AssertThat(pipe.writePipe, Equals(nullptr));
		});
	});
});
