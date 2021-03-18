// Copyright 2015-2021 Piperift - All rights reserved

#include <Reflection/TypeId.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;
using namespace Rift::Refl;

struct One
{};


go_bandit([]() {
	describe("Reflection.TypeId", []() {
		it("Different types don't share an id", [&]() {
			static constexpr TypeId ids[]{
			    TypeId::Get<u8>(), TypeId::Get<u16>(), TypeId::Get<i32>(), TypeId::Get<One>()};
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
