// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Files/PlatformPaths.h>


using namespace p;


Spec("Files.Paths", []()
{
It("Can get root name and path", []()
{
#if P_PLATFORM_WINDOWS
	Expect(p::GetRootPathName("F:\\SomeFolder\\AnotherFolder")).ToEqual("F:");
	Expect(p::GetRootPath("F:\\SomeFolder\\AnotherFolder")).ToEqual("F:\\");
#elif P_PLATFORM_LINUX
	Expect(p::GetRootPathName("/var/SomeFolder/AnotherFolder")).ToEqual("");
	Expect(p::GetRootPath("/var/SomeFolder/AnotherFolder")).ToEqual("/");
#endif
	Expect(p::GetRootPathName("/AnotherFolder")).ToEqual("");
	Expect(p::GetRootPath("/AnotherFolder")).ToEqual("/");
});

It("Can get relative path", []()
{
#if P_PLATFORM_WINDOWS
	Expect(p::GetRelativePath("F:\\SomeFolder\\AnotherFolder")).ToEqual("SomeFolder\\AnotherFolder");
#endif
	Expect(p::GetRelativePath("/var/SomeFolder/AnotherFolder")).ToEqual("var/SomeFolder/AnotherFolder");
	Expect(p::GetRelativePath("/SomeFolder/AnotherFolder")).ToEqual("SomeFolder/AnotherFolder");
});

It("Can check absolute path", []()
{
	Expect(p::IsAbsolutePath("//host")).ToEqual(true);
#if P_PLATFORM_WINDOWS
	Expect(p::IsAbsolutePath("F:\\SomeFolder\\AnotherFolder")).ToEqual(true);
#elif P_PLATFORM_LINUX
	Expect(p::IsAbsolutePath("/var/SomeFolder/AnotherFolder")).ToEqual(true);
#endif
	Expect(p::IsAbsolutePath("Executable.exe")).ToEqual(false);
	Expect(p::IsAbsolutePath("SomeFolder/AnotherFolder")).ToEqual(false);
});

It("Can check relative path", []()
{
#if P_PLATFORM_WINDOWS
	Expect(p::IsRelativePath("F:\\SomeFolder\\AnotherFolder")).ToEqual(false);
#elif P_PLATFORM_LINUX
	Expect(p::IsRelativePath("/var/SomeFolder/AnotherFolder")).ToEqual(false);
#endif
	Expect(p::IsRelativePath("Executable.exe")).ToEqual(true);
	Expect(p::IsRelativePath("SomeFolder/AnotherFolder")).ToEqual(true);
});

It("Can get parent path", []()
{
#if P_PLATFORM_WINDOWS
	Expect(p::GetParentPath("F:\\SomeFolder\\AnotherFolder")).ToEqual("F:\\SomeFolder");
#endif
	Expect(p::GetParentPath("/var/SomeFolder")).ToEqual("/var");
	Expect(p::GetParentPath("/SomeFolder/AnotherFolder")).ToEqual("/SomeFolder");
	Expect(p::GetParentPath("/SomeFolder/SomeFile.txt")).ToEqual("/SomeFolder");
});

It("Executable path is not empty", []()
{
	Expect(p::PlatformPaths::GetExecutablePath()).ToNotEqual("");
});

It("Can get extension", []()
{
#if P_PLATFORM_WINDOWS
	Expect(p::GetExtension("F:\\SomeFolder\\AnotherFolder.lib")).ToEqual(".lib");
	Expect(p::GetExtension("F:\\AnotherFolder.lib")).ToEqual(".lib");
	Expect(p::GetExtension("F:\\AnotherFolder.")).ToEqual(".");
	Expect(p::GetExtension("F:\\AnotherFolder")).ToEqual("");
	Expect(p::GetExtension("F:\\")).ToEqual("");
#elif P_PLATFORM_LINUX
	Expect(p::GetExtension("/var/SomeFolder/AnotherFolder.lib")).ToEqual(".lib");
	Expect(p::GetExtension("/var/AnotherFolder.lib")).ToEqual(".lib");
	Expect(p::GetExtension("/var/AnotherFolder.")).ToEqual(".");
	Expect(p::GetExtension("/var/AnotherFolder")).ToEqual("");
	Expect(p::GetExtension("/var/")).ToEqual("");
#endif
	Expect(p::GetExtension("AnotherFolder.lib")).ToEqual(".lib");
	Expect(p::GetExtension("AnotherFolder")).ToEqual("");
});

It("Can check extension", []()
{
#if P_PLATFORM_WINDOWS
	Expect(p::HasExtension("F:\\SomeFolder\\AnotherFolder.lib")).ToEqual(true);
	Expect(p::HasExtension("F:\\AnotherFolder.lib")).ToEqual(true);
	Expect(p::HasExtension("F:\\AnotherFolder.")).ToEqual(true);
	Expect(p::HasExtension("F:\\AnotherFolder")).ToEqual(false);
	Expect(p::HasExtension("F:\\")).ToEqual(false);
#elif P_PLATFORM_LINUX
	Expect(p::HasExtension("/var/SomeFolder/AnotherFolder.lib")).ToEqual(true);
	Expect(p::HasExtension("/var/AnotherFolder.lib")).ToEqual(true);
	Expect(p::HasExtension("/var/AnotherFolder.")).ToEqual(true);
	Expect(p::HasExtension("/var/AnotherFolder")).ToEqual(false);
	Expect(p::HasExtension("/var/")).ToEqual(false);
#endif
	Expect(p::HasExtension("AnotherFolder.lib")).ToEqual(true);
	Expect(p::HasExtension("AnotherFolder")).ToEqual(false);
});

It("Can replace extension", []()
{
	p::String path;
#if P_PLATFORM_WINDOWS
	path = "F:\\SomeFolder\\AnotherFolder.lib";
	p::ReplaceExtension(path, "txt");
	Expect(path).ToEqual("F:\\SomeFolder\\AnotherFolder.txt");
#elif P_PLATFORM_LINUX
	path = "/var/SomeFolder/AnotherFolder.lib";
	p::ReplaceExtension(path, "txt");
	Expect(path).ToEqual("/var/SomeFolder/AnotherFolder.txt");
#endif
	path = "AnotherFolder.lib";
	p::ReplaceExtension(path, "txt");
	Expect(path).ToEqual("AnotherFolder.txt");
	path = "AnotherFolder.";
	p::ReplaceExtension(path, ".txt");
	Expect(path).ToEqual("AnotherFolder.txt");
	path = "AnotherFolder.lib";
	p::ReplaceExtension(path, ".txt");
	Expect(path).ToEqual("AnotherFolder.txt");
	path = "AnotherFolder";
	p::ReplaceExtension(path, "txt");
	Expect(path).ToEqual("AnotherFolder.txt");
});

It("Can get stem", []()
{
#if P_PLATFORM_WINDOWS
	Expect(p::GetStem("F:\\SomeFolder\\AnotherFolder.lib")).ToEqual("AnotherFolder");
	Expect(p::GetStem("F:\\AnotherFolder.lib")).ToEqual("AnotherFolder");
	Expect(p::GetStem("F:\\AnotherFolder.")).ToEqual("AnotherFolder");
	Expect(p::GetStem("F:\\AnotherFolder")).ToEqual("AnotherFolder");
	Expect(p::GetStem("F:\\")).ToEqual("");
#elif P_PLATFORM_LINUX
	Expect(p::GetStem("/var/SomeFolder/AnotherFolder.lib")).ToEqual("AnotherFolder");
	Expect(p::GetStem("/var/AnotherFolder.lib")).ToEqual("AnotherFolder");
	Expect(p::GetStem("/var/AnotherFolder.")).ToEqual("AnotherFolder");
	Expect(p::GetStem("/var/AnotherFolder")).ToEqual("AnotherFolder");
	Expect(p::GetStem("/var/")).ToEqual("");
#endif
	Expect(p::GetStem("AnotherFolder.lib")).ToEqual("AnotherFolder");
	Expect(p::GetStem("AnotherFolder")).ToEqual("AnotherFolder");
	Expect(p::GetStem("")).ToEqual("");
});

It("Can check stem", []()
{
#if P_PLATFORM_WINDOWS
	Expect(p::HasStem("F:\\SomeFolder\\AnotherFolder.lib")).ToEqual(true);
	Expect(p::HasStem("F:\\AnotherFolder.lib")).ToEqual(true);
	Expect(p::HasStem("F:\\AnotherFolder.")).ToEqual(true);
	Expect(p::HasStem("F:\\AnotherFolder")).ToEqual(true);
	Expect(p::HasStem("F:\\")).ToEqual(false);
#elif P_PLATFORM_LINUX
	Expect(p::HasStem("/var/SomeFolder/AnotherFolder.lib")).ToEqual(true);
	Expect(p::HasStem("/var/AnotherFolder.lib")).ToEqual(true);
	Expect(p::HasStem("/var/AnotherFolder.")).ToEqual(true);
	Expect(p::HasStem("/var/AnotherFolder")).ToEqual(true);
	Expect(p::HasStem("/var/")).ToEqual(false);
#endif
	Expect(p::HasStem("AnotherFolder.lib")).ToEqual(true);
	Expect(p::HasStem("AnotherFolder")).ToEqual(true);
	Expect(p::HasStem("")).ToEqual(false);
});


It("Can append to path", []()
{
	Expect(p::JoinPaths("", "")).ToEqual("");
	Expect(p::JoinPaths("", "/")).ToEqual("/");
	Expect(p::JoinPaths("", "bar")).ToEqual("bar");
	Expect(p::JoinPaths("", "/bar")).ToEqual("/bar");

	Expect(p::JoinPaths("/", "")).ToEqual("/");
	Expect(p::JoinPaths("/", "/")).ToEqual("/");
	Expect(p::JoinPaths("/", "bar")).ToEqual("/bar");
	Expect(p::JoinPaths("/", "/bar")).ToEqual("/bar");
	Expect(p::JoinPaths("foo", "/")).ToEqual("/");

	Expect(p::JoinPaths("foo", "/bar")).ToEqual("/bar");
	Expect(p::JoinPaths("foo/", "")).ToEqual("foo/");
	Expect(p::JoinPaths("foo/", "/")).ToEqual("/");
	Expect(p::JoinPaths("foo/", "bar")).ToEqual("foo/bar");

#if P_PLATFORM_WINDOWS
	Expect(p::JoinPaths("foo", "")).ToEqual("foo\\");
	Expect(p::JoinPaths("foo", "bar")).ToEqual("foo\\bar");
	Expect(p::JoinPaths("foo\\", "\\bar")).ToEqual("\\bar");
	Expect(p::JoinPaths("c:", "bar")).ToEqual("c:bar");
	Expect(p::JoinPaths("\\\\host", "foo")).ToEqual("\\\\host\\foo");
	Expect(p::JoinPaths("\\\\host/", "foo")).ToEqual("\\\\host/foo");
#else
	Expect(p::JoinPaths("foo", "")).ToEqual("foo/");
	Expect(p::JoinPaths("foo", "bar")).ToEqual("foo/bar");
	Expect(p::JoinPaths("//host", "foo")).ToEqual("//host/foo");
	Expect(p::JoinPaths("//host/", "foo")).ToEqual("//host/foo");
#endif
});
});
