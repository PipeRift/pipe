// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTests.h>
#include <Pipe/Core/TypeTraits.h>
#include <PipeReflect.h>
#include <PipeSerialize.h>


using namespace p;


struct TestNotSerializable
{};

struct TestSerializable
{
	void ReadProperties(p::Reader& r) {}
	void WriteProperties(p::Writer& w) const {}
};

struct TestWithSuper : public TestNotSerializable
{
	using Super = TestNotSerializable;
};

struct TestExternal : public TestNotSerializable
{};

void Read(p::Reader& r, TestExternal& v) {}
void Write(p::Writer& r, const TestExternal& v) {}

namespace p
{
	struct TestExternal2 : public TestNotSerializable
	{};

	void Read(Reader& r, TestExternal2& v) {}
	void Write(Writer& r, const TestExternal2& v) {}
}    // namespace p


void RegisterReflectionTraitsTests()
{
	Spec("Reflection.Traits", []()
	{
		Describe("Read/Write properties", []()
		{
			It("Can check for read properties", []()
			{
				Expect(p::HasReadProperties<TestNotSerializable>()).ToBeFalse();
				Expect(p::HasReadProperties<TestSerializable>()).ToBeTrue();
				Expect(p::Readable<TestNotSerializable>).ToBeFalse();
				Expect(p::Readable<TestSerializable>).ToBeTrue();
			});

			It("Can check for write properties", []()
			{
				Expect(p::HasWriteProperties<TestNotSerializable>()).ToBeFalse();
				Expect(p::HasWriteProperties<TestSerializable>()).ToBeTrue();
				Expect(p::Writable<TestNotSerializable>).ToBeFalse();
				Expect(p::Writable<TestSerializable>).ToBeTrue();
			});
		});

		Describe("Read/Write external", []()
		{
			It("Can check for read properties", []()
			{
				Expect(p::Readable<TestNotSerializable>).ToBeFalse();
				Expect(p::Readable<TestExternal>).ToBeTrue();
			});

			It("Can check for write properties", []()
			{
				Expect(p::Writable<TestNotSerializable>).ToBeFalse();
				Expect(p::Writable<TestExternal>).ToBeTrue();
			});
		});

		Describe("Read/Write external in namespace", []()
		{
			It("Can check for read properties", []()
			{
				Expect(p::Readable<p::TestExternal2>).ToBeTrue();
			});

			It("Can check for write properties", []()
			{
				Expect(p::Writable<p::TestExternal2>).ToBeTrue();
			});
		});

		It("Can check super", []()
		{
			Expect(p::HasSuper<TestNotSerializable>()).ToBeFalse();
			Expect(p::HasSuper<TestWithSuper>()).ToBeTrue();
		});

		It("Can build type on Arrays", []()
		{
			Expect(p::CanBuildType<p::TArray<bool>>()).ToBeTrue();
			Expect(p::HasExternalBuildType<p::TArray<bool>>()).ToBeTrue();
		});
	});
}
