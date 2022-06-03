// Copyright 2015-2022 Piperift - All rights reserved

#include "bandit/grammar.h"

#include <bandit/bandit.h>
#include <ECS/Context.h>
#include <ECS/Filtering.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;
using namespace Rift::ECS;

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


struct Type
{};
struct TypeTwo
{};
struct TypeThree
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
			ctx.Add<Type>(id1);
			ctx.Add<Type, TypeTwo, TypeThree>(id2);
			ctx.Add<TypeTwo, TypeThree>(id3);
			ctx.Add<TypeTwo, TypeThree>(id4);
			ctx.Add<TypeTwo>(id5);
		});

		describe("ListAny/ListAll", [&]() {
			it("Can get list matching all", [&]() {
				TAccess<Type, TypeTwo, TypeThree> access{ctx};
				TArray<ECS::Id> typeIds = ECS::ListAll<Type>(access);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());

				TArray<ECS::Id> type2Ids = ECS::ListAll<TypeTwo, TypeThree>(access);
				AssertThat(type2Ids.Contains(id1), Is().False());
				AssertThat(type2Ids.Contains(id2), Is().True());
				AssertThat(type2Ids.Contains(id3), Is().True());
			});

			it("Can get list matching any", [&]() {
				TAccess<Type, TypeTwo, TypeThree> access{ctx};
				TArray<ECS::Id> typeIds = ECS::ListAny<Type>(access);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());

				TArray<ECS::Id> type2Ids = ECS::ListAny<Type, TypeThree>(access);
				AssertThat(type2Ids.Contains(id1), Is().True());
				AssertThat(type2Ids.Contains(id2), Is().True());
				AssertThat(type2Ids.Contains(id3), Is().True());
			});

			it("Doesn't list removed ids", [&]() {
				TAccess<TypeTwo> access{ctx};
				ctx.Destroy(id2);    // Remove first in the pool
				ctx.Destroy(id3);    // Remove last in the pool
				ctx.Destroy(id4);    // Remove last in the pool

				TArray<ECS::Id> ids = ECS::ListAll<TypeTwo>(access);
				AssertThat(ids.Contains(ECS::NoId), Is().False());
				AssertThat(ids.Size(), Equals(1));
			});
		});

		describe("RemoveIf", [&]() {
			it("Removes ids containing component", [&]() {
				TAccess<Type, TypeTwo, TypeThree> access{ctx};
				TArray<ECS::Id> typeIds = ECS::ListAny<Type>(access);

				ECS::RemoveIf<TypeThree>(access, typeIds);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().False());
				AssertThat(typeIds.Contains(id3), Is().False());
			});

			it("Removes ids not containing component", [&]() {
				TAccess<Type, TypeTwo, TypeThree> access{ctx};
				TArray<ECS::Id> typeIds = ECS::ListAny<Type>(access);

				ECS::RemoveIfNot<TypeThree>(access, typeIds);
				AssertThat(typeIds.Contains(id1), Is().False());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());
			});

			it("Removes ids containing multiple component", [&]() {
				TAccess<Type, TypeTwo, TypeThree> access{ctx};
				TArray<ECS::Id> typeIds = ECS::ListAny<Type, TypeTwo, TypeThree>(access);

				ECS::RemoveIf<TypeTwo, TypeThree>(access, typeIds);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().False());
				AssertThat(typeIds.Contains(id3), Is().False());
			});
		});

		describe("GetIf", [&]() {
			it("Finds ids containing a component from a list", [&]() {
				TArray<ECS::Id> source{id1, id2, id3};

				TAccess<Type> access{ctx};
				TArray<ECS::Id> typeIds = ECS::GetIf<Type>(access, source);
				AssertThat(typeIds.Contains(id1), Is().True());
				AssertThat(typeIds.Contains(id2), Is().True());
				AssertThat(typeIds.Contains(id3), Is().False());
			});

			it("Finds ids not containing a component from a list", [&]() {
				TArray<ECS::Id> source{id1, id2, id3};

				TAccess<Type> access{ctx};
				TArray<ECS::Id> ids = ECS::GetIfNot<Type>(access, source);
				AssertThat(ids.Contains(id1), Is().False());
				AssertThat(ids.Contains(id2), Is().False());
				AssertThat(ids.Contains(id3), Is().True());
			});
		});

		describe("ExtractIf", [&]() {
			it("Finds and removes ids containing a component from a list", [&]() {
				TArray<ECS::Id> source{id1, id2, id3};

				TAccess<Type> access{ctx};
				TArray<ECS::Id> ids = ECS::ExtractIf<Type>(access, source);
				AssertThat(ids.Contains(id1), Is().True());
				AssertThat(ids.Contains(id2), Is().True());
				AssertThat(ids.Contains(id3), Is().False());
				AssertThat(source.Contains(id1), Is().False());
				AssertThat(source.Contains(id2), Is().False());
				AssertThat(source.Contains(id3), Is().True());
			});

			it("Finds and removes ids not containing a component from a list", [&]() {
				TArray<ECS::Id> source{id1, id2, id3};

				TAccess<Type> access{ctx};
				TArray<ECS::Id> ids = ECS::ExtractIfNot<Type>(access, source);
				AssertThat(ids.Contains(id1), Is().False());
				AssertThat(ids.Contains(id2), Is().False());
				AssertThat(ids.Contains(id3), Is().True());
				AssertThat(source.Contains(id1), Is().True());
				AssertThat(source.Contains(id2), Is().True());
				AssertThat(source.Contains(id3), Is().False());
			});
		});

		it("Can filter directly from ECS", [&]() {
			TArray<ECS::Id> ids1 = ECS::ListAll<Type>(ctx);
			AssertThat(ids1.Contains(id1), Is().True());

			TArray<ECS::Id> ids2 = ECS::ListAny<Type>(ctx);
			AssertThat(ids2.Contains(id1), Is().True());

			TArray<ECS::Id> ids3 = ECS::ListAny<Type>(ctx);
			ECS::RemoveIf<TypeThree>(ctx, ids3);
			AssertThat(ids3.Contains(id1), Is().True());

			TArray<ECS::Id> ids4 = ECS::ListAny<Type>(ctx);
			ECS::RemoveIfNot<TypeThree>(ctx, ids4);
			AssertThat(ids4.Contains(id1), Is().False());
		});
	});
});
