// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <bandit/bandit.h>
#include <PipeArrays.h>
#include <PipeReflect.h>


using namespace snowhouse;
using namespace bandit;


struct TestStruct
{
	P_STRUCT(TestStruct)

	P_PROP(value0)
	bool value0 = true;

	P_PROP(value1)
	p::TArray<float> value1 = true;
};


go_bandit([]() {
	describe("Reflection.Macros", []() {
		it("Can get property names", [&]() {
			p::TypeId testStructType = p::RegisterTypeId<TestStruct>();

			AssertThat(p::HasTypeFlags(testStructType, p::TF_Struct), Equals(true));

			auto properties = p::GetTypeProperties(testStructType);
			AssertThat(properties.Size(), Equals(2));

			// AssertThat(properties[0].typeId, Equals(p::GetTypeId<p::TArray<float>>()));
			AssertThat(properties[0]->name.Data(), Equals("value0"));
			// AssertThat(properties[1].typeId, Equals(p::GetTypeId<bool>()));
			AssertThat(properties[1]->name.Data(), Equals("value1"));
		});
	});
});
