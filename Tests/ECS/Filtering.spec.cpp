// Copyright 2015-2022 Piperift - All rights reserved

#include "bandit/grammar.h"

#include <bandit/bandit.h>
#include <PECS/Context.h>
#include <PECS/Filtering.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;
using namespace p::ecs;

namespace snowhouse
{
	template<>
	struct Stringizer<ecs::Id>
	{
		static std::string ToString(ecs::Id id)
		{
			std::stringstream stream;
			stream << "Id(" << UnderlyingType<ecs::Id>(id) << ")";
			return stream.str();
		}
	};
}    // namespace snowhouse


struct TypeA
{};
struct TypeB
{};
struct TypeC
{};


go_bandit([]() {
	ecs::Context ctx;
	ecs::Id id1;
	ecs::Id id2;
	ecs::Id id3;
	ecs::Id id4;
	ecs::Id id5;
	describe("ECS.Filtering", [&]() {
		before_each([&]() {
			ctx = {};
			id1 = ctx.Create();
			id2 = ctx.Create();
			id3 = ctx.Create();
			id4 = ctx.Create();
			id5 = ctx.Create();
			ctx.Add<TypeA>(id1);
			ctx.Add<TypeA, TypeB, TypeC>(id2);
			ctx.Add<TypeB, TypeC>(id3);
			ctx.Add<TypeB, TypeC>(id4);
			ctx.Add<TypeB>(id5);
		});

		describe("ListAny/ListAll", [&]() {
			it("Can get list matching all", [&]() {
				TAccess<TypeA, TypeB, TypeC> access{ctx};
				TArray<ecs::Id> typeIds = ecs::ListAll<TypeA>(access);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());

				TArray<ecs::Id> type2Ids = ecs::ListAll<TypeB, TypeC>(access);
				AssertThat(type2Ids.Contains(id1), Is().False());
				AssertThat(type2Ids.Contains(id2), Is().True());
				AssertThat(type2Ids.Contains(id3), Is().True());
			});

			it("Can get list matching any", [&]() {
				TAccess<TypeA, TypeB, TypeC> access{ctx};
				TArray<ecs::Id> typeIds = ecs::ListAny<TypeA>(access);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());

				TArray<ecs::Id> type2Ids = ecs::ListAny<TypeA, TypeC>(access);
				AssertThat(type2Ids.Contains(id1), Is().True());
				AssertThat(type2Ids.Contains(id2), Is().True());
				AssertThat(type2Ids.Contains(id3), Is().True());
			});

			it("Doesn't list removed ids", [&]() {
				TAccess<TypeB> access{ctx};
				ctx.Destroy(id2);    // Remove first in the pool
				ctx.Destroy(id3);    // Remove last in the pool
				ctx.Destroy(id4);    // Remove last in the pool

				TArray<ecs::Id> ids = ecs::ListAll<TypeB>(access);
				AssertThat(ids.Contains(ecs::NoId), Is().False());
				AssertThat(ids.Size(), Equals(1));
			});
		});

		describe("ExcludeIf", [&]() {
			it("Removes ids containing component", [&]() {
				TAccess<TypeA, TypeB, TypeC> access{ctx};
				TArray<ecs::Id> typeIds = ecs::ListAny<TypeA>(access);

				ecs::ExcludeIf<TypeC>(access, typeIds);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().False());
				AssertThat(typeIds.Contains(id3), Is().False());
			});

			it("Removes ids not containing component", [&]() {
				TAccess<TypeA, TypeB, TypeC> access{ctx};
				TArray<ecs::Id> typeIds = ecs::ListAny<TypeA>(access);

				ecs::ExcludeIfNot<TypeC>(access, typeIds);
				AssertThat(typeIds.Contains(id1), Is().False());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());
			});

			it("Removes ids containing multiple component", [&]() {
				TAccess<TypeA, TypeB, TypeC> access{ctx};
				TArray<ecs::Id> typeIds = ecs::ListAny<TypeA, TypeB, TypeC>(access);

				ecs::ExcludeIf<TypeB, TypeC>(access, typeIds);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().False());
				AssertThat(typeIds.Contains(id3), Is().False());
			});
		});

		describe("GetIf", [&]() {
			it("Finds ids containing a component from a list", [&]() {
				TArray<ecs::Id> source{id1, id2, id3};

				TAccess<TypeA> access{ctx};
				TArray<ecs::Id> typeIds = ecs::GetIf<TypeA>(access, source);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());
			});

			it("Finds ids not containing a component from a list", [&]() {
				TArray<ecs::Id> source{id1, id2, id3};

				TAccess<TypeA> access{ctx};
				TArray<ecs::Id> ids = ecs::GetIfNot<TypeA>(access, source);
				AssertThat(ids.Contains(id1), Is().False());
				AssertThat(ids.Contains(id2), Is().False());
				AssertThat(ids.Contains(id3), Is().True());
			});
		});

		describe("ExtractIf", [&]() {
			it("Finds and removes ids containing a component from a list", [&]() {
				TArray<ecs::Id> source{id1, id2, id3};

				TAccess<TypeA> access{ctx};
				TArray<ecs::Id> ids = ecs::ExtractIf<TypeA>(access, source);
				AssertThat(ids.Contains(id1), Is().True());
				AssertThat(ids.Contains(id2), Is().True());
				AssertThat(ids.Contains(id3), Is().False());
				AssertThat(source.Contains(id1), Is().False());
				AssertThat(source.Contains(id2), Is().False());
				AssertThat(source.Contains(id3), Is().True());
			});

			it("Finds and removes ids not containing a component from a list", [&]() {
				TArray<ecs::Id> source{id1, id2, id3};

				TAccess<TypeA> access{ctx};
				TArray<ecs::Id> ids = ecs::ExtractIfNot<TypeA>(access, source);
				AssertThat(ids.Contains(id1), Is().False());
				AssertThat(ids.Contains(id2), Is().False());
				AssertThat(ids.Contains(id3), Is().True());
				AssertThat(source.Contains(id1), Is().True());
				AssertThat(source.Contains(id2), Is().True());
				AssertThat(source.Contains(id3), Is().False());
			});
		});

		it("Can filter directly from ECS", [&]() {
			TArray<ecs::Id> ids1 = ecs::ListAll<TypeA>(ctx);
			AssertThat(ids1.Contains(id1), Is().True());

			TArray<ecs::Id> ids2 = ecs::ListAny<TypeA>(ctx);
			AssertThat(ids2.Contains(id1), Is().True());

			TArray<ecs::Id> ids3 = ecs::ListAny<TypeA>(ctx);
			ecs::ExcludeIf<TypeC>(ctx, ids3);
			AssertThat(ids3.Contains(id1), Is().True());

			TArray<ecs::Id> ids4 = ecs::ListAny<TypeA>(ctx);
			ecs::ExcludeIfNot<TypeC>(ctx, ids4);
			AssertThat(ids4.Contains(id1), Is().False());
		});
	});
});
