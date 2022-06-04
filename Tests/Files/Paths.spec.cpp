// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Files/Paths.h>
#include <Platform/PlatformProcess.h>


using namespace snowhouse;
using namespace bandit;


go_bandit([]() {
	describe("Files.Paths", []() {
		it("Can get root name and path", [&]() {
#if PLATFORM_WINDOWS
			AssertThat(Pipe::GetRootPathName("F:\\SomeFolder\\AnotherFolder"), Equals("F:"));
			AssertThat(Pipe::GetRootPath("F:\\SomeFolder\\AnotherFolder"), Equals("F:\\"));
#elif PLATFORM_LINUX
			AssertThat(Pipe::GetRootPathName("/var/SomeFolder/AnotherFolder"), Equals(""));
			AssertThat(Pipe::GetRootPath("/var/SomeFolder/AnotherFolder"), Equals("/"));
#endif
			AssertThat(Pipe::GetRootPathName("/AnotherFolder"), Equals(""));
			AssertThat(Pipe::GetRootPath("/AnotherFolder"), Equals("/"));
		});

		it("Can get relative path", [&]() {
#if PLATFORM_WINDOWS
			AssertThat(Pipe::GetRelativePath("F:\\SomeFolder\\AnotherFolder"),
			    Equals("SomeFolder\\AnotherFolder"));
#endif
			AssertThat(Pipe::GetRelativePath("/var/SomeFolder/AnotherFolder"),
			    Equals("var/SomeFolder/AnotherFolder"));
			AssertThat(Pipe::GetRelativePath("/SomeFolder/AnotherFolder"),
			    Equals("SomeFolder/AnotherFolder"));
		});

		it("Can get parent path", [&]() {
#if PLATFORM_WINDOWS
			AssertThat(
			    Pipe::GetParentPath("F:\\SomeFolder\\AnotherFolder"), Equals("F:\\SomeFolder"));
#endif
			AssertThat(Pipe::GetParentPath("/var/SomeFolder"), Equals("/var"));
			AssertThat(Pipe::GetParentPath("/SomeFolder/AnotherFolder"), Equals("/SomeFolder"));
			AssertThat(Pipe::GetParentPath("/SomeFolder/SomeFile.txt"), Equals("/SomeFolder"));
		});

		it("Executable path is not empty", [&]() {
			AssertThat(Pipe::PlatformProcess::GetExecutablePath(), !Equals(""));
		});
	});
});
