// Copyright 2015-2021 Piperift - All rights reserved

#include <Assets/AssetPtr.h>
#include <Containers/Array.h>
#include <Containers/Map.h>
#include <Reflection/PredefinedTypes.h>
#include <Reflection/TypeName.h>
#include <Strings/String.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;
using namespace Rift::Refl;


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
			AssertThat(GetTypeName<TCHAR>(), Equals("TCHAR"));
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

		it("Can get TAssetPtr names", [&]() {
			AssertThat(GetTypeName<TAssetPtr<AssetData>>(), Equals("TAssetPtr"));
			AssertThat(
			    GetFullTypeName<TAssetPtr<AssetData>>(), Equals("TAssetPtr<Rift::AssetData>"));
		});

		describe("Containers", []() {
			it("Can get TArray names", [&]() {
				AssertThat(GetTypeName<TArray<bool>>(), Equals("TArray"));
				AssertThat(GetFullTypeName<TArray<bool>>(), Equals("TArray<bool>"));
			});

			it("Can get TMap names", [&]() {
				auto name = GetTypeName<TMap<u8, bool>>();
				AssertThat(name, Equals("TMap"));

				auto fullName = GetFullTypeName<TMap<u8, bool>>();
				AssertThat(fullName, Equals("TMap<u8, bool>"));
			});
		});
	});
});
