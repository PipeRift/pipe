// Copyright 2015-2021 Piperift - All rights reserved

#include <Log.h>
#include <Context.h>
#include <Files/Paths.h>
#include <Platform/PlatformProcess.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;


go_bandit([]() {
	describe("Files.Paths", []() {
		it("Can get root name and path", [&]() {
#if PLATFORM_WINDOWS
			AssertThat(Paths::GetRootName("F:\\SomeFolder\\AnotherFolder"), Equals("F:"));
			AssertThat(Paths::GetRoot("F:\\SomeFolder\\AnotherFolder"), Equals("F:\\"));
#elif PLATFORM_LINUX
			AssertThat(Paths::GetRootName("/var/SomeFolder/AnotherFolder"), Equals(""));
			AssertThat(Paths::GetRoot("/var/SomeFolder/AnotherFolder"), Equals("/"));
#endif
			AssertThat(Paths::GetRootName("/AnotherFolder"), Equals(""));
			AssertThat(Paths::GetRoot("/AnotherFolder"), Equals("/"));
		});

		it("Can get relative path", [&]() {
#if PLATFORM_WINDOWS
			AssertThat(Paths::GetRelative("F:\\SomeFolder\\AnotherFolder"),
			    Equals("SomeFolder\\AnotherFolder"));
#endif
			AssertThat(Paths::GetRelative("/var/SomeFolder/AnotherFolder"),
			    Equals("var/SomeFolder/AnotherFolder"));
			AssertThat(Paths::GetRelative("/SomeFolder/AnotherFolder"),
			    Equals("SomeFolder/AnotherFolder"));
		});
	});
});
