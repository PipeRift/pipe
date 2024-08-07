// Copyright 2015-2024 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/Core/Guid.h>
#include <Pipe/Core/Map.h>
#include <Pipe/Core/String.h>
#include <Pipe/Core/Tag.h>
#include <PipeArrays.h>
#include <PipeReflect.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


struct AnStruct
{};

class AClass
{};

namespace Space
{
	struct Other
	{};
}    // namespace Space


go_bandit([]() {
	describe("Reflection.TypeName", []() {
		it("Can get Platform type names", [&]() {
			AssertThat(GetTypeName<u8>(), Equals("u8"));
			AssertThat(GetTypeName<u16>(), Equals("u16"));
			AssertThat(GetTypeName<u32>(), Equals("u32"));
			AssertThat(GetTypeName<u64>(), Equals("u64"));
			AssertThat(GetTypeName<i8>(), Equals("i8"));
			AssertThat(GetTypeName<i16>(), Equals("i16"));
			AssertThat(GetTypeName<i32>(), Equals("i32"));
			AssertThat(GetTypeName<i64>(), Equals("i64"));
			AssertThat(GetTypeName<char>(), Equals("TChar"));
			AssertThat(GetTypeName<StringView>(), Equals("StringView"));
			AssertThat(GetTypeName<String>(), Equals("String"));
		});

		it("Can get Native type names", [&]() {
			AssertThat(GetTypeName<bool>(), Equals("bool"));
			AssertThat(GetTypeName<float>(), Equals("float"));
			AssertThat(GetTypeName<double>(), Equals("double"));
		});

		it("Can get Class names", [&]() {
			AssertThat(GetTypeName<AClass>(), Equals("AClass"));
		});

		it("Can get Struct names", [&]() {
			AssertThat(GetTypeName<AnStruct>(), Equals("AnStruct"));
		});

		it("Can get names with namespaces", [&]() {
			AssertThat(GetTypeName<Space::Other>(), Equals("Space::Other"));
		});

		describe("Containers", []() {
			it("Can get TArray names", [&]() {
				AssertThat(GetTypeName<TArray<Guid>>(), Equals("TArray"));
				AssertThat(GetFullTypeName<TArray<Guid>>(), Equals("TArray<p::Guid>"));
				AssertThat(GetFullTypeName<TArray<Guid>>(false), Equals("TArray<Guid>"));
			});

			it("Can get TMap names", [&]() {
				auto name = GetTypeName<TMap<u8, bool>>();
				AssertThat(name, Equals("TMap"));

				auto fullName = GetFullTypeName<TMap<u8, bool>>();
				AssertThat(fullName, Equals("TMap<u8, bool>"));


				auto namespaceName = GetFullTypeName<TMap<u8, Guid>>();
				AssertThat(namespaceName, Equals("TMap<u8, p::Guid>"));
				auto noNamespaceName = GetFullTypeName<TMap<u8, Guid>>(false);
				AssertThat(noNamespaceName, Equals("TMap<u8, Guid>"));
			});
		});
	});
});
