// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTests.h>
#include <PipeContainers.h>
#include <PipeReflect.h>


using namespace p;


struct TestStruct
{
	P_STRUCT(TestStruct)

	P_PROP(value0)
	bool value0 = true;

	P_PROP(value1)
	p::TArray<float> value1 = true;
};


void RegisterReflectionMacroReflectionTests()
{
	Spec("Reflection.Macros", []()
	{
		It("Can get property names", []()
		{
			p::TypeId testStructType = p::RegisterTypeId<TestStruct>();

			Expect(p::HasTypeFlags(testStructType, p::TF_Struct)).ToEqual(true);

			auto properties = p::GetTypeProperties(testStructType);
			Expect(properties.Size()).ToEqual(2);

			// Expect(properties[0].typeId).ToEqual(p::GetTypeId<p::TArray<float>>());
			Expect(properties[0]->name.Data()).ToEqual("value0");
			// Expect(properties[1].typeId).ToEqual(p::GetTypeId<bool>());
			Expect(properties[1]->name.Data()).ToEqual("value1");
		});
	});
}
