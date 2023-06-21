// Copyright 2015-2023 Piperift - All rights reserved

#include "bandit/grammar.h"

#include <bandit/bandit.h>
#include <Pipe/PipeECS.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;

namespace snowhouse
{
	template<>
	struct Stringizer<Id>
	{
		static std::string ToString(Id id)
		{
			std::stringstream stream;
			stream << "Id(" << UnderlyingType<Id>(id) << ")";
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
	EntityContext ctx;
	Id id1;
	Id id2;
	Id id3;
	Id id4;
	Id id5;
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

		describe("FindAllIdsWith/FindAllIdsWithAny", [&]() {
			it("Can get list matching all", [&]() {
				TAccess<TypeA, TypeB, TypeC> access{ctx};
				TArray<Id> typeIds = FindAllIdsWith<TypeA>(access);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());

				TArray<Id> type2Ids = FindAllIdsWith<TypeB, TypeC>(access);
				AssertThat(type2Ids.Contains(id1), Is().False());
				AssertThat(type2Ids.Contains(id2), Is().True());
				AssertThat(type2Ids.Contains(id3), Is().True());
			});

			it("Can get list matching any", [&]() {
				TAccess<TypeA, TypeB, TypeC> access{ctx};
				TArray<Id> typeIds = FindAllIdsWithAny<TypeA>(access);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());

				TArray<Id> type2Ids = FindAllIdsWithAny<TypeA, TypeC>(access);
				AssertThat(type2Ids.Contains(id1), Is().True());
				AssertThat(type2Ids.Contains(id2), Is().True());
				AssertThat(type2Ids.Contains(id3), Is().True());
			});

			it("Doesn't list removed ids", [&]() {
				TAccess<TypeB> access{ctx};
				ctx.Destroy(id2);    // Remove first in the pool
				ctx.Destroy(id3);    // Remove last in the pool
				ctx.Destroy(id4);    // Remove last in the pool

				TArray<Id> ids = FindAllIdsWith<TypeB>(access);
				AssertThat(ids.Contains(NoId), Is().False());
				AssertThat(ids.Size(), Equals(1));
			});
		});

		describe("ExcludeIdsWith", [&]() {
			it("Removes ids containing component", [&]() {
				TAccess<TypeA, TypeB, TypeC> access{ctx};
				TArray<Id> typeIds = FindAllIdsWithAny<TypeA>(access);

				ExcludeIdsWith<TypeC>(access, typeIds);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().False());
				AssertThat(typeIds.Contains(id3), Is().False());
			});

			it("Removes ids not containing component", [&]() {
				TAccess<TypeA, TypeB, TypeC> access{ctx};
				TArray<Id> typeIds = FindAllIdsWithAny<TypeA>(access);

				ExcludeIdsWithout<TypeC>(access, typeIds);
				AssertThat(typeIds.Contains(id1), Is().False());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());
			});

			it("Removes ids containing multiple component", [&]() {
				TAccess<TypeA, TypeB, TypeC> access{ctx};
				TArray<Id> typeIds = FindAllIdsWithAny<TypeA, TypeB, TypeC>(access);

				ExcludeIdsWith<TypeB, TypeC>(access, typeIds);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().False());
				AssertThat(typeIds.Contains(id3), Is().False());
			});
		});

		describe("FindIdsWith", [&]() {
			it("Finds ids containing a component from a list", [&]() {
				TArray<Id> source{id1, id2, id3};

				TAccess<TypeA> access{ctx};
				TArray<Id> typeIds = FindIdsWith<TypeA>(access, source);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());
			});

			it("Finds ids not containing a component from a list", [&]() {
				TArray<Id> source{id1, id2, id3};

				TAccess<TypeA> access{ctx};
				TArray<Id> ids = FindIdsWithout<TypeA>(access, source);
				AssertThat(ids.Contains(id1), Is().False());
				AssertThat(ids.Contains(id2), Is().False());
				AssertThat(ids.Contains(id3), Is().True());
			});
		});

		describe("ExtractIdsWith", [&]() {
			it("Finds and removes ids containing a component from a list", [&]() {
				TArray<Id> source{id1, id2, id3};

				TAccess<TypeA> access{ctx};
				TArray<Id> ids = ExtractIdsWith<TypeA>(access, source);
				AssertThat(ids.Contains(id1), Is().True());
				AssertThat(ids.Contains(id2), Is().True());
				AssertThat(ids.Contains(id3), Is().False());
				AssertThat(source.Contains(id1), Is().False());
				AssertThat(source.Contains(id2), Is().False());
				AssertThat(source.Contains(id3), Is().True());
			});

			it("Finds and removes ids not containing a component from a list", [&]() {
				TArray<Id> source{id1, id2, id3};

				TAccess<TypeA> access{ctx};
				TArray<Id> ids = ExtractIdsWithout<TypeA>(access, source);
				AssertThat(ids.Contains(id1), Is().False());
				AssertThat(ids.Contains(id2), Is().False());
				AssertThat(ids.Contains(id3), Is().True());
				AssertThat(source.Contains(id1), Is().True());
				AssertThat(source.Contains(id2), Is().True());
				AssertThat(source.Contains(id3), Is().False());
			});
		});

		it("Can filter directly from ECS", [&]() {
			TArray<Id> ids1 = FindAllIdsWith<TypeA>(ctx);
			AssertThat(ids1.Contains(id1), Is().True());

			TArray<Id> ids2 = FindAllIdsWithAny<TypeA>(ctx);
			AssertThat(ids2.Contains(id1), Is().True());

			TArray<Id> ids3 = FindAllIdsWithAny<TypeA>(ctx);
			ExcludeIdsWith<TypeC>(ctx, ids3);
			AssertThat(ids3.Contains(id1), Is().True());

			TArray<Id> ids4 = FindAllIdsWithAny<TypeA>(ctx);
			ExcludeIdsWithout<TypeC>(ctx, ids4);
			AssertThat(ids4.Contains(id1), Is().False());
		});
	});
});
