// Copyright 2015-2022 Piperift - All rights reserved

#include "bandit/grammar.h"

#include <bandit/bandit.h>
#include <ECS/Context.h>
#include <ECS/Filtering.h>


using namespace snowhouse;
using namespace bandit;
using namespace Pipe;
using namespace Pipe::ECS;

namespace snowhouse
{
	template<>
	struct Stringizer<ECS::Id>
	{
		static std::string ToString(ECS::Id id)
		{
			std::stringstream stream;
			stream << "Id(" << UnderlyingType<ECS::Id>(id) << ")";
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
	ECS::Context ctx;
	ECS::Id id1;
	ECS::Id id2;
	ECS::Id id3;
	ECS::Id id4;
	ECS::Id id5;
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
				TArray<ECS::Id> typeIds = ECS::ListAll<TypeA>(access);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());

				TArray<ECS::Id> type2Ids = ECS::ListAll<TypeB, TypeC>(access);
				AssertThat(type2Ids.Contains(id1), Is().False());
				AssertThat(type2Ids.Contains(id2), Is().True());
				AssertThat(type2Ids.Contains(id3), Is().True());
			});

			it("Can get list matching any", [&]() {
				TAccess<TypeA, TypeB, TypeC> access{ctx};
				TArray<ECS::Id> typeIds = ECS::ListAny<TypeA>(access);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());

				TArray<ECS::Id> type2Ids = ECS::ListAny<TypeA, TypeC>(access);
				AssertThat(type2Ids.Contains(id1), Is().True());
				AssertThat(type2Ids.Contains(id2), Is().True());
				AssertThat(type2Ids.Contains(id3), Is().True());
			});

			it("Doesn't list removed ids", [&]() {
				TAccess<TypeB> access{ctx};
				ctx.Destroy(id2);    // Remove first in the pool
				ctx.Destroy(id3);    // Remove last in the pool
				ctx.Destroy(id4);    // Remove last in the pool

				TArray<ECS::Id> ids = ECS::ListAll<TypeB>(access);
				AssertThat(ids.Contains(ECS::NoId), Is().False());
				AssertThat(ids.Size(), Equals(1));
			});
		});

		describe("ExcludeIf", [&]() {
			it("Removes ids containing component", [&]() {
				TAccess<TypeA, TypeB, TypeC> access{ctx};
				TArray<ECS::Id> typeIds = ECS::ListAny<TypeA>(access);

				ECS::ExcludeIf<TypeC>(access, typeIds);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().False());
				AssertThat(typeIds.Contains(id3), Is().False());
			});

			it("Removes ids not containing component", [&]() {
				TAccess<TypeA, TypeB, TypeC> access{ctx};
				TArray<ECS::Id> typeIds = ECS::ListAny<TypeA>(access);

				ECS::ExcludeIfNot<TypeC>(access, typeIds);
				AssertThat(typeIds.Contains(id1), Is().False());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());
			});

			it("Removes ids containing multiple component", [&]() {
				TAccess<TypeA, TypeB, TypeC> access{ctx};
				TArray<ECS::Id> typeIds = ECS::ListAny<TypeA, TypeB, TypeC>(access);

				ECS::ExcludeIf<TypeB, TypeC>(access, typeIds);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().False());
				AssertThat(typeIds.Contains(id3), Is().False());
			});
		});

		describe("GetIf", [&]() {
			it("Finds ids containing a component from a list", [&]() {
				TArray<ECS::Id> source{id1, id2, id3};

				TAccess<TypeA> access{ctx};
				TArray<ECS::Id> typeIds = ECS::GetIf<TypeA>(access, source);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());
			});

			it("Finds ids not containing a component from a list", [&]() {
				TArray<ECS::Id> source{id1, id2, id3};

				TAccess<TypeA> access{ctx};
				TArray<ECS::Id> ids = ECS::GetIfNot<TypeA>(access, source);
				AssertThat(ids.Contains(id1), Is().False());
				AssertThat(ids.Contains(id2), Is().False());
				AssertThat(ids.Contains(id3), Is().True());
			});
		});

		describe("ExtractIf", [&]() {
			it("Finds and removes ids containing a component from a list", [&]() {
				TArray<ECS::Id> source{id1, id2, id3};

				TAccess<TypeA> access{ctx};
				TArray<ECS::Id> ids = ECS::ExtractIf<TypeA>(access, source);
				AssertThat(ids.Contains(id1), Is().True());
				AssertThat(ids.Contains(id2), Is().True());
				AssertThat(ids.Contains(id3), Is().False());
				AssertThat(source.Contains(id1), Is().False());
				AssertThat(source.Contains(id2), Is().False());
				AssertThat(source.Contains(id3), Is().True());
			});

			it("Finds and removes ids not containing a component from a list", [&]() {
				TArray<ECS::Id> source{id1, id2, id3};

				TAccess<TypeA> access{ctx};
				TArray<ECS::Id> ids = ECS::ExtractIfNot<TypeA>(access, source);
				AssertThat(ids.Contains(id1), Is().False());
				AssertThat(ids.Contains(id2), Is().False());
				AssertThat(ids.Contains(id3), Is().True());
				AssertThat(source.Contains(id1), Is().True());
				AssertThat(source.Contains(id2), Is().True());
				AssertThat(source.Contains(id3), Is().False());
			});
		});

		it("Can filter directly from ECS", [&]() {
			TArray<ECS::Id> ids1 = ECS::ListAll<TypeA>(ctx);
			AssertThat(ids1.Contains(id1), Is().True());

			TArray<ECS::Id> ids2 = ECS::ListAny<TypeA>(ctx);
			AssertThat(ids2.Contains(id1), Is().True());

			TArray<ECS::Id> ids3 = ECS::ListAny<TypeA>(ctx);
			ECS::ExcludeIf<TypeC>(ctx, ids3);
			AssertThat(ids3.Contains(id1), Is().True());

			TArray<ECS::Id> ids4 = ECS::ListAny<TypeA>(ctx);
			ECS::ExcludeIfNot<TypeC>(ctx, ids4);
			AssertThat(ids4.Contains(id1), Is().False());
		});
	});
});
