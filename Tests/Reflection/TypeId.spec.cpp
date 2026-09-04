// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTests.h>
#include <PipeReflect.h>


using namespace p;

struct One
{};


void RegisterReflectionTypeIdTests()
{
	Spec("Reflection.TypeId", []()
	{
		It("Ids can be valid and invalid", []()
		{
			static constexpr TypeId id = GetTypeId<u8>();
			Expect(id.IsValid()).ToEqual(true);

			static constexpr TypeId noId{};
			Expect(noId.IsValid()).ToEqual(false);
		});

		It("Different types don't share an id", []()
		{
			static constexpr TypeId ids[]{
			    GetTypeId<u8>(), GetTypeId<u16>(), GetTypeId<i32>(), GetTypeId<One>()};
			static constexpr u32 numIds = sizeof(ids) / sizeof(TypeId);

			// Check that no id matches the other
			for (u32 i = 0; i < numIds; ++i)
			{
				for (u32 e = i + 1; e < numIds; ++e)
				{
					Expect(ids[i]).ToNotEqual(ids[e]);
				}
			}
		});
	});
}
