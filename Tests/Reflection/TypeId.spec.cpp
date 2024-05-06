// Copyright 2015-2024 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/Reflect/TypeId.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;

struct One
{};


go_bandit([]() {
	describe("Reflection.TypeId", []() {
		it("Ids can be valid and invalid", [&]() {
			static constexpr TypeId id = GetTypeId<u8>();
			AssertThat(id.IsValid(), Equals(true));

			static constexpr TypeId noId{};
			AssertThat(noId.IsValid(), Equals(false));
		});

		it("Different types don't share an id", [&]() {
			static constexpr TypeId ids[]{
			    GetTypeId<u8>(), GetTypeId<u16>(), GetTypeId<i32>(), GetTypeId<One>()};
			static constexpr u32 numIds = sizeof(ids) / sizeof(TypeId);

			// Check that no id matches the other
			for (u32 i = 0; i < numIds; ++i)
			{
				for (u32 e = i + 1; e < numIds; ++e)
				{
					AssertThat(ids[i], !Equals(ids[e]));
				}
			}
		});
	});
});
