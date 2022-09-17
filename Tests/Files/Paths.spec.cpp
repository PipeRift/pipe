// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/Core/PlatformProcess.h>
#include <Pipe/Files/Paths.h>


using namespace snowhouse;
using namespace bandit;


go_bandit([]() {
	describe("Files.Paths", []() {
		it("Can get root name and path", [&]() {
#if P_PLATFORM_WINDOWS
			AssertThat(p::GetRootPathName("F:\\SomeFolder\\AnotherFolder"), Equals("F:"));
			AssertThat(p::GetRootPath("F:\\SomeFolder\\AnotherFolder"), Equals("F:\\"));
#elif P_PLATFORM_LINUX
			AssertThat(p::GetRootPathName("/var/SomeFolder/AnotherFolder"), Equals(""));
			AssertThat(p::GetRootPath("/var/SomeFolder/AnotherFolder"), Equals("/"));
#endif
			AssertThat(p::GetRootPathName("/AnotherFolder"), Equals(""));
			AssertThat(p::GetRootPath("/AnotherFolder"), Equals("/"));
		});

		it("Can get relative path", [&]() {
#if P_PLATFORM_WINDOWS
			AssertThat(p::GetRelativePath("F:\\SomeFolder\\AnotherFolder"),
			    Equals("SomeFolder\\AnotherFolder"));
#endif
			AssertThat(p::GetRelativePath("/var/SomeFolder/AnotherFolder"),
			    Equals("var/SomeFolder/AnotherFolder"));
			AssertThat(p::GetRelativePath("/SomeFolder/AnotherFolder"),
			    Equals("SomeFolder/AnotherFolder"));
		});

		it("Can get parent path", [&]() {
#if P_PLATFORM_WINDOWS
			AssertThat(p::GetParentPath("F:\\SomeFolder\\AnotherFolder"), Equals("F:\\SomeFolder"));
#endif
			AssertThat(p::GetParentPath("/var/SomeFolder"), Equals("/var"));
			AssertThat(p::GetParentPath("/SomeFolder/AnotherFolder"), Equals("/SomeFolder"));
			AssertThat(p::GetParentPath("/SomeFolder/SomeFile.txt"), Equals("/SomeFolder"));
		});

		it("Executable path is not empty", [&]() {
			AssertThat(p::PlatformProcess::GetExecutablePath(), !Equals(""));
		});
	});
});
