// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <Pipe/Core/Guid.h>
#include <Pipe/Core/Map.h>
#include <Pipe/Core/Tag.h>
#include <PipeContainers.h>
#include <PipeReflect.h>
#include <PipeStrings.h>


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


namespace
{
// Auto-registers via static init (macro-free go_bandit equivalent).
const bool autoRegistered = []()
{
Spec("Reflection.TypeName", []()
{
	It("Can get Platform type names", []()
	{
		Expect(GetTypeName<u8>()).ToEqual("u8");
		Expect(GetTypeName<u16>()).ToEqual("u16");
		Expect(GetTypeName<u32>()).ToEqual("u32");
		Expect(GetTypeName<u64>()).ToEqual("u64");
		Expect(GetTypeName<i8>()).ToEqual("i8");
		Expect(GetTypeName<i16>()).ToEqual("i16");
		Expect(GetTypeName<i32>()).ToEqual("i32");
		Expect(GetTypeName<i64>()).ToEqual("i64");
		Expect(GetTypeName<char>()).ToEqual("char");
		Expect(GetTypeName<StringView>()).ToEqual("StringView");
		Expect(GetTypeName<String>()).ToEqual("String");
	});

	It("Can get Native type names", []()
	{
		Expect(GetTypeName<bool>()).ToEqual("bool");
		Expect(GetTypeName<float>()).ToEqual("float");
		Expect(GetTypeName<double>()).ToEqual("double");
	});

	It("Can get Class names", []()
	{
		Expect(GetTypeName<AClass>()).ToEqual("AClass");
	});

	It("Can get Struct names", []()
	{
		Expect(GetTypeName<AnStruct>()).ToEqual("AnStruct");
	});

	It("Can get names with namespaces", []()
	{
		Expect(GetTypeName<Space::Other>()).ToEqual("Space::Other");
	});

	Describe("Containers", []()
	{
		It("Can get TArray names", []()
		{
			Expect(GetTypeName<TArray<Guid>>()).ToEqual("TArray");
			Expect(GetFullTypeName<TArray<Guid>>()).ToEqual("TArray<p::Guid>");
			Expect(GetFullTypeName<TArray<Guid>>(false)).ToEqual("TArray<Guid>");
		});

		It("Can get TMap names", []()
		{
			auto name = GetTypeName<TMap<u8, bool>>();
			Expect(name).ToEqual("TMap");

			auto fullName = GetFullTypeName<TMap<u8, bool>>();
			Expect(fullName).ToEqual("TMap<u8, bool>");


			auto namespaceName = GetFullTypeName<TMap<u8, Guid>>();
			Expect(namespaceName).ToEqual("TMap<u8, p::Guid>");
			auto noNamespaceName = GetFullTypeName<TMap<u8, Guid>>(false);
			Expect(noNamespaceName).ToEqual("TMap<u8, Guid>");
		});
	});
});
return true;
}();
}    // namespace
