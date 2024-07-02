// Copyright 2015-2024 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <PipeReflect.h>


using namespace snowhouse;
using namespace bandit;


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


go_bandit([]() {
	describe("Reflection.Traits", []() {
		describe("Read/Write properties", []() {
			it("Can check for read properties", [&]() {
				AssertThat(p::HasReadProperties<TestNotSerializable>(), Is().False());
				AssertThat(p::HasReadProperties<TestSerializable>(), Is().True());
				AssertThat(p::Readable<TestNotSerializable>, Is().False());
				AssertThat(p::Readable<TestSerializable>, Is().True());
			});

			it("Can check for write properties", [&]() {
				AssertThat(p::HasWriteProperties<TestNotSerializable>(), Is().False());
				AssertThat(p::HasWriteProperties<TestSerializable>(), Is().True());
				AssertThat(p::Writable<TestNotSerializable>, Is().False());
				AssertThat(p::Writable<TestSerializable>, Is().True());
			});
		});

		describe("Read/Write external", []() {
			it("Can check for read properties", [&]() {
				AssertThat(p::Readable<TestNotSerializable>, Is().False());
				AssertThat(p::Readable<TestExternal>, Is().True());
			});

			it("Can check for write properties", [&]() {
				AssertThat(p::Writable<TestNotSerializable>, Is().False());
				AssertThat(p::Writable<TestExternal>, Is().True());
			});
		});

		describe("Read/Write external in namespace", []() {
			it("Can check for read properties", [&]() {
				AssertThat(p::Readable<p::TestExternal2>, Is().True());
			});

			it("Can check for write properties", [&]() {
				AssertThat(p::Writable<p::TestExternal2>, Is().True());
			});
		});

		it("Can check super", []() {
			AssertThat(p::HasSuper<TestNotSerializable>(), Is().False());
			AssertThat(p::HasSuper<TestWithSuper>(), Is().True());
		});

		it("Can build type on Arrays", []() {
			AssertThat(p::CanBuildType<p::TArray<bool>>(), Is().True());
			AssertThat(p::HasExternalBuildType<p::TArray<bool>>(), Is().True());
		});
	});
});
