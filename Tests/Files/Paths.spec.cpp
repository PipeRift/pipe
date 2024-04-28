// Copyright 2015-2024 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Files/PlatformPaths.h>


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

		it("Can check absolute path", [&]() {
			AssertThat(p::IsAbsolutePath("//host"), Equals(true));
#if P_PLATFORM_WINDOWS
			AssertThat(p::IsAbsolutePath("F:\\SomeFolder\\AnotherFolder"), Equals(true));
#elif P_PLATFORM_LINUX
			AssertThat(p::IsAbsolutePath("/var/SomeFolder/AnotherFolder"), Equals(true));
#endif
			AssertThat(p::IsAbsolutePath("Executable.exe"), Equals(false));
			AssertThat(p::IsAbsolutePath("SomeFolder/AnotherFolder"), Equals(false));
		});

		it("Can check relative path", [&]() {
#if P_PLATFORM_WINDOWS
			AssertThat(p::IsRelativePath("F:\\SomeFolder\\AnotherFolder"), Equals(false));
#elif P_PLATFORM_LINUX
			AssertThat(p::IsRelativePath("/var/SomeFolder/AnotherFolder"), Equals(false));
#endif
			AssertThat(p::IsRelativePath("Executable.exe"), Equals(true));
			AssertThat(p::IsRelativePath("SomeFolder/AnotherFolder"), Equals(true));
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
			AssertThat(p::PlatformPaths::GetExecutablePath(), !Equals(""));
		});

		it("Can get extension", [&]() {
#if P_PLATFORM_WINDOWS
			AssertThat(p::GetExtension("F:\\SomeFolder\\AnotherFolder.lib"), Equals(".lib"));
			AssertThat(p::GetExtension("F:\\AnotherFolder.lib"), Equals(".lib"));
			AssertThat(p::GetExtension("F:\\AnotherFolder."), Equals("."));
			AssertThat(p::GetExtension("F:\\AnotherFolder"), Equals(""));
			AssertThat(p::GetExtension("F:\\"), Equals(""));
#elif P_PLATFORM_LINUX
			AssertThat(p::GetExtension("/var/SomeFolder/AnotherFolder.lib"), Equals(".lib"));
			AssertThat(p::GetExtension("/var/AnotherFolder.lib"), Equals(".lib"));
			AssertThat(p::GetExtension("/var/AnotherFolder."), Equals("."));
			AssertThat(p::GetExtension("/var/AnotherFolder"), Equals(""));
			AssertThat(p::GetExtension("/var/"), Equals(""));
#endif
			AssertThat(p::GetExtension("AnotherFolder.lib"), Equals(".lib"));
			AssertThat(p::GetExtension("AnotherFolder"), Equals(""));
		});

		it("Can check extension", [&]() {
#if P_PLATFORM_WINDOWS
			AssertThat(p::HasExtension("F:\\SomeFolder\\AnotherFolder.lib"), Equals(true));
			AssertThat(p::HasExtension("F:\\AnotherFolder.lib"), Equals(true));
			AssertThat(p::HasExtension("F:\\AnotherFolder."), Equals(true));
			AssertThat(p::HasExtension("F:\\AnotherFolder"), Equals(false));
			AssertThat(p::HasExtension("F:\\"), Equals(false));
#elif P_PLATFORM_LINUX
			AssertThat(p::HasExtension("/var/SomeFolder/AnotherFolder.lib"), Equals(true));
			AssertThat(p::HasExtension("/var/AnotherFolder.lib"), Equals(true));
			AssertThat(p::HasExtension("/var/AnotherFolder."), Equals(true));
			AssertThat(p::HasExtension("/var/AnotherFolder"), Equals(false));
			AssertThat(p::HasExtension("/var/"), Equals(false));
#endif
			AssertThat(p::HasExtension("AnotherFolder.lib"), Equals(true));
			AssertThat(p::HasExtension("AnotherFolder"), Equals(false));
		});

		it("Can replace extension", [&]() {
			p::String path;
#if P_PLATFORM_WINDOWS
			path = "F:\\SomeFolder\\AnotherFolder.lib";
			p::ReplaceExtension(path, "txt");
			AssertThat(path, Equals(p::String{"F:\\SomeFolder\\AnotherFolder.txt"}));
#elif P_PLATFORM_LINUX
			path = "/var/SomeFolder/AnotherFolder.lib";
			p::ReplaceExtension(path, "txt");
			AssertThat(path, Equals(p::String{"/var/SomeFolder/AnotherFolder.txt"}));
#endif
			path = "AnotherFolder.lib";
			p::ReplaceExtension(path, "txt");
			AssertThat(path, Equals(p::String{"AnotherFolder.txt"}));
			path = "AnotherFolder.";
			p::ReplaceExtension(path, ".txt");
			AssertThat(path, Equals(p::String{"AnotherFolder.txt"}));
			path = "AnotherFolder.lib";
			p::ReplaceExtension(path, ".txt");
			AssertThat(path, Equals(p::String{"AnotherFolder.txt"}));
			path = "AnotherFolder";
			p::ReplaceExtension(path, "txt");
			AssertThat(path, Equals(p::String{"AnotherFolder.txt"}));
		});

		it("Can get stem", [&]() {
#if P_PLATFORM_WINDOWS
			AssertThat(p::GetStem("F:\\SomeFolder\\AnotherFolder.lib"), Equals("AnotherFolder"));
			AssertThat(p::GetStem("F:\\AnotherFolder.lib"), Equals("AnotherFolder"));
			AssertThat(p::GetStem("F:\\AnotherFolder."), Equals("AnotherFolder"));
			AssertThat(p::GetStem("F:\\AnotherFolder"), Equals("AnotherFolder"));
			AssertThat(p::GetStem("F:\\"), Equals(""));
#elif P_PLATFORM_LINUX
			AssertThat(p::GetStem("/var/SomeFolder/AnotherFolder.lib"), Equals("AnotherFolder"));
			AssertThat(p::GetStem("/var/AnotherFolder.lib"), Equals("AnotherFolder"));
			AssertThat(p::GetStem("/var/AnotherFolder."), Equals("AnotherFolder"));
			AssertThat(p::GetStem("/var/AnotherFolder"), Equals("AnotherFolder"));
			AssertThat(p::GetStem("/var/"), Equals(""));
#endif
			AssertThat(p::GetStem("AnotherFolder.lib"), Equals("AnotherFolder"));
			AssertThat(p::GetStem("AnotherFolder"), Equals("AnotherFolder"));
			AssertThat(p::GetStem(""), Equals(""));
		});

		it("Can check stem", [&]() {
#if P_PLATFORM_WINDOWS
			AssertThat(p::HasStem("F:\\SomeFolder\\AnotherFolder.lib"), Equals(true));
			AssertThat(p::HasStem("F:\\AnotherFolder.lib"), Equals(true));
			AssertThat(p::HasStem("F:\\AnotherFolder."), Equals(true));
			AssertThat(p::HasStem("F:\\AnotherFolder"), Equals(true));
			AssertThat(p::HasStem("F:\\"), Equals(false));
#elif P_PLATFORM_LINUX
			AssertThat(p::HasStem("/var/SomeFolder/AnotherFolder.lib"), Equals(true));
			AssertThat(p::HasStem("/var/AnotherFolder.lib"), Equals(true));
			AssertThat(p::HasStem("/var/AnotherFolder."), Equals(true));
			AssertThat(p::HasStem("/var/AnotherFolder"), Equals(true));
			AssertThat(p::HasStem("/var/"), Equals(false));
#endif
			AssertThat(p::HasStem("AnotherFolder.lib"), Equals(true));
			AssertThat(p::HasStem("AnotherFolder"), Equals(true));
			AssertThat(p::HasStem(""), Equals(false));
		});


		it("Can append to path", [&]() {
			AssertThat(p::JoinPaths("", ""), Equals(p::StringView{""}));
			AssertThat(p::JoinPaths("", "/"), Equals(p::StringView{"/"}));
			AssertThat(p::JoinPaths("", "bar"), Equals(p::StringView{"bar"}));
			AssertThat(p::JoinPaths("", "/bar"), Equals(p::StringView{"/bar"}));

			AssertThat(p::JoinPaths("/", ""), Equals(p::StringView{"/"}));
			AssertThat(p::JoinPaths("/", "/"), Equals(p::StringView{"/"}));
			AssertThat(p::JoinPaths("/", "bar"), Equals(p::StringView{"/bar"}));
			AssertThat(p::JoinPaths("/", "/bar"), Equals(p::StringView{"/bar"}));
			AssertThat(p::JoinPaths("foo", "/"), Equals(p::StringView{"/"}));

			AssertThat(p::JoinPaths("foo", "/bar"), Equals(p::StringView{"/bar"}));
			AssertThat(p::JoinPaths("foo/", ""), Equals(p::StringView{"foo/"}));
			AssertThat(p::JoinPaths("foo/", "/"), Equals(p::StringView{"/"}));
			AssertThat(p::JoinPaths("foo/", "bar"), Equals(p::StringView{"foo/bar"}));

#if P_PLATFORM_WINDOWS
			AssertThat(p::JoinPaths("foo", ""), Equals(p::StringView{"foo\\"}));
			AssertThat(p::JoinPaths("foo", "bar"), Equals(p::StringView{"foo\\bar"}));
			AssertThat(p::JoinPaths("foo\\", "\\bar"), Equals(p::StringView{"\\bar"}));
			AssertThat(p::JoinPaths("c:", "bar"), Equals(p::StringView{"c:bar"}));
			AssertThat(p::JoinPaths("\\\\host", "foo"), Equals(p::String{"\\\\host\\foo"}));
			AssertThat(p::JoinPaths("\\\\host/", "foo"), Equals(p::String{"\\\\host/foo"}));
#else
			AssertThat(p::JoinPaths("foo", ""), Equals(p::StringView{"foo/"}));
			AssertThat(p::JoinPaths("foo", "bar"), Equals(p::StringView{"foo/bar"}));
			AssertThat(p::JoinPaths("//host", "foo"), Equals(p::StringView{"//host/foo"}));
			AssertThat(p::JoinPaths("//host/", "foo"), Equals(p::StringView{"//host/foo"}));
#endif
		});
	});
});
