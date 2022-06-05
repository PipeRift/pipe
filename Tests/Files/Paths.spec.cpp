// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Core/PlatformProcess.h>
#include <Files/Paths.h>


using namespace snowhouse;
using namespace bandit;


go_bandit([]() {
	describe("Files.Paths", []() {
		it("Can get root name and path", [&]() {
#if PLATFORM_WINDOWS
			AssertThat(pipe::GetRootPathName("F:\\SomeFolder\\AnotherFolder"), Equals("F:"));
			AssertThat(pipe::GetRootPath("F:\\SomeFolder\\AnotherFolder"), Equals("F:\\"));
#elif PLATFORM_LINUX
			AssertThat(pipe::GetRootPathName("/var/SomeFolder/AnotherFolder"), Equals(""));
			AssertThat(pipe::GetRootPath("/var/SomeFolder/AnotherFolder"), Equals("/"));
#endif
			AssertThat(pipe::GetRootPathName("/AnotherFolder"), Equals(""));
			AssertThat(pipe::GetRootPath("/AnotherFolder"), Equals("/"));
		});

		it("Can get relative path", [&]() {
#if PLATFORM_WINDOWS
			AssertThat(pipe::GetRelativePath("F:\\SomeFolder\\AnotherFolder"),
			    Equals("SomeFolder\\AnotherFolder"));
#endif
			AssertThat(pipe::GetRelativePath("/var/SomeFolder/AnotherFolder"),
			    Equals("var/SomeFolder/AnotherFolder"));
			AssertThat(pipe::GetRelativePath("/SomeFolder/AnotherFolder"),
			    Equals("SomeFolder/AnotherFolder"));
		});

		it("Can get parent path", [&]() {
#if PLATFORM_WINDOWS
			AssertThat(
			    pipe::GetParentPath("F:\\SomeFolder\\AnotherFolder"), Equals("F:\\SomeFolder"));
#endif
			AssertThat(pipe::GetParentPath("/var/SomeFolder"), Equals("/var"));
			AssertThat(pipe::GetParentPath("/SomeFolder/AnotherFolder"), Equals("/SomeFolder"));
			AssertThat(pipe::GetParentPath("/SomeFolder/SomeFile.txt"), Equals("/SomeFolder"));
		});

		it("Executable path is not empty", [&]() {
			AssertThat(pipe::PlatformProcess::GetExecutablePath(), !Equals(""));
		});
	});
});
