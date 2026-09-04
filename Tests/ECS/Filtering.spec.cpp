// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <PipeECS.h>


using namespace p;


struct TypeA
{};
struct TypeB
{};
struct TypeC
{};


namespace
{
	IdContext ctx;
	Id id1;
	Id id2;
	Id id3;
	Id id4;
	Id id5;
}    // namespace


namespace
{
// Auto-registers via static init (macro-free go_bandit equivalent).
const bool autoRegistered = []()
{
Spec("ECS.Filtering", []()
{
	BeforeEach([]()
	{
		ctx = {};
		id1 = AddId(ctx);
		id2 = AddId(ctx);
		id3 = AddId(ctx);
		id4 = AddId(ctx);
		id5 = AddId(ctx);
		ctx.Add<TypeA>(id1);
		ctx.Add<TypeA, TypeB, TypeC>(id2);
		ctx.Add<TypeB, TypeC>(id3);
		ctx.Add<TypeB, TypeC>(id4);
		ctx.Add<TypeB>(id5);
	});

	Describe("FindAllIdsWith/FindAllIdsWithAny", []()
	{
		It("Can get list matching all", []()
		{
			TIdScope<TypeA, TypeB, TypeC> access{ctx};
			TArray<Id> typeIds = FindAllIdsWith<TypeA>(access);
			Expect(typeIds.Contains(id1)).ToBeTrue();
			Expect(typeIds.Contains(id2)).ToBeTrue();
			Expect(typeIds.Contains(id3)).ToBeFalse();

			TArray<Id> type2Ids = FindAllIdsWith<TypeB, TypeC>(access);
			Expect(type2Ids.Contains(id1)).ToBeFalse();
			Expect(type2Ids.Contains(id2)).ToBeTrue();
			Expect(type2Ids.Contains(id3)).ToBeTrue();
		});

		It("Can get list matching any", []()
		{
			TIdScope<TypeA, TypeB, TypeC> access{ctx};
			TArray<Id> typeIds = FindAllIdsWithAny<TypeA>(access);
			Expect(typeIds.Contains(id1)).ToBeTrue();
			Expect(typeIds.Contains(id2)).ToBeTrue();
			Expect(typeIds.Contains(id3)).ToBeFalse();

			TArray<Id> type2Ids = FindAllIdsWithAny<TypeA, TypeC>(access);
			Expect(type2Ids.Contains(id1)).ToBeTrue();
			Expect(type2Ids.Contains(id2)).ToBeTrue();
			Expect(type2Ids.Contains(id3)).ToBeTrue();
		});

		It("Doesn't list removed ids", []()
		{
			TIdScope<TypeB> access{ctx};
			RmId(ctx, id2, RmIdFlags::Instant);
			RmId(ctx, id3, RmIdFlags::Instant);
			RmId(ctx, id4, RmIdFlags::Instant);

			TArray<Id> ids = FindAllIdsWith<TypeB>(access);
			Expect(ids.Contains(NoId)).ToBeFalse();
			Expect(ids.Size()).ToEqual(1);
		});

		It("Doesn't list (deferred) removed ids", []()
		{
			TIdScope<TypeB> access{ctx};
			RmId(ctx, id2);
			RmId(ctx, id3);
			RmId(ctx, id4);

			FlushDeferredRemovals(ctx);

			TArray<Id> ids = FindAllIdsWith<TypeB>(access);
			Expect(ids.Contains(NoId)).ToBeFalse();
			Expect(ids.Size()).ToEqual(1);
		});
	});

	Describe("ExcludeIdsWith", []()
	{
		It("Removes ids containing component", []()
		{
			TIdScope<TypeA, TypeB, TypeC> access{ctx};
			TArray<Id> typeIds = FindAllIdsWithAny<TypeA>(access);

			ExcludeIdsWith<TypeC>(access, typeIds);
			Expect(typeIds.Contains(id1)).ToBeTrue();
			Expect(typeIds.Contains(id2)).ToBeFalse();
			Expect(typeIds.Contains(id3)).ToBeFalse();
		});

		It("Removes ids not containing component", []()
		{
			TIdScope<TypeA, TypeB, TypeC> access{ctx};
			TArray<Id> typeIds = FindAllIdsWithAny<TypeA>(access);

			ExcludeIdsWithout<TypeC>(access, typeIds);
			Expect(typeIds.Contains(id1)).ToBeFalse();
			Expect(typeIds.Contains(id2)).ToBeTrue();
			Expect(typeIds.Contains(id3)).ToBeFalse();
		});

		It("Removes ids containing multiple component", []()
		{
			TIdScope<TypeA, TypeB, TypeC> access{ctx};
			TArray<Id> typeIds = FindAllIdsWithAny<TypeA, TypeB, TypeC>(access);

			ExcludeIdsWith<TypeB, TypeC>(access, typeIds);
			Expect(typeIds.Contains(id1)).ToBeTrue();
			Expect(typeIds.Contains(id2)).ToBeFalse();
			Expect(typeIds.Contains(id3)).ToBeFalse();
		});
	});

	Describe("FindIdsWith", []()
	{
		It("Finds ids containing a component from a list", []()
		{
			TArray<Id> source{id1, id2, id3};

			TIdScope<TypeA> access{ctx};
			TArray<Id> typeIds = FindIdsWith<TypeA>(access, source);
			Expect(typeIds.Contains(id1)).ToBeTrue();
			Expect(typeIds.Contains(id2)).ToBeTrue();
			Expect(typeIds.Contains(id3)).ToBeFalse();
		});

		It("Finds ids not containing a component from a list", []()
		{
			TArray<Id> source{id1, id2, id3};

			TIdScope<TypeA> access{ctx};
			TArray<Id> ids = FindIdsWithout<TypeA>(access, source);
			Expect(ids.Contains(id1)).ToBeFalse();
			Expect(ids.Contains(id2)).ToBeFalse();
			Expect(ids.Contains(id3)).ToBeTrue();
		});
	});

	Describe("ExtractIdsWith", []()
	{
		It("Finds and removes ids containing a component from a list", []()
		{
			TArray<Id> source{id1, id2, id3};

			TIdScope<TypeA> access{ctx};
			TArray<Id> ids = ExtractIdsWith<TypeA>(access, source);
			Expect(ids.Contains(id1)).ToBeTrue();
			Expect(ids.Contains(id2)).ToBeTrue();
			Expect(ids.Contains(id3)).ToBeFalse();
			Expect(source.Contains(id1)).ToBeFalse();
			Expect(source.Contains(id2)).ToBeFalse();
			Expect(source.Contains(id3)).ToBeTrue();
		});

		It("Finds and removes ids not containing a component from a list", []()
		{
			TArray<Id> source{id1, id2, id3};

			TIdScope<TypeA> access{ctx};
			TArray<Id> ids = ExtractIdsWithout<TypeA>(access, source);
			Expect(ids.Contains(id1)).ToBeFalse();
			Expect(ids.Contains(id2)).ToBeFalse();
			Expect(ids.Contains(id3)).ToBeTrue();
			Expect(source.Contains(id1)).ToBeTrue();
			Expect(source.Contains(id2)).ToBeTrue();
			Expect(source.Contains(id3)).ToBeFalse();
		});
	});

	It("Can filter directly from ECS", []()
	{
		TArray<Id> ids1 = FindAllIdsWith<TypeA>(ctx);
		Expect(ids1.Contains(id1)).ToBeTrue();

		TArray<Id> ids2 = FindAllIdsWithAny<TypeA>(ctx);
		Expect(ids2.Contains(id1)).ToBeTrue();

		TArray<Id> ids3 = FindAllIdsWithAny<TypeA>(ctx);
		ExcludeIdsWith<TypeC>(ctx, ids3);
		Expect(ids3.Contains(id1)).ToBeTrue();

		TArray<Id> ids4 = FindAllIdsWithAny<TypeA>(ctx);
		ExcludeIdsWithout<TypeC>(ctx, ids4);
		Expect(ids4.Contains(id1)).ToBeFalse();
	});

	It("Can filter CRemoved", []()
	{
		RmId(ctx, id1);
		RmId(ctx, id2);
		RmId(ctx, id3);

		TArray<Id> ids1 = FindAllIdsWith<TypeA>(ctx);
		Expect(ids1.Contains(id1)).ToBeTrue();
		TArray<Id> ids2 = FindAllIdsWith<CRemoved>(ctx);
		Expect(ids2.Contains(id1)).ToBeTrue();
		Expect(ids2.Contains(id2)).ToBeTrue();
		Expect(ids2.Contains(id3)).ToBeTrue();
		Expect(ids2.Size()).ToEqual(3);

		TArray<Id> ids3 = FindAllIdsWith<CRemoved, TypeA>(ctx);
		Expect(ids3.Contains(id1)).ToBeTrue();
		Expect(ids3.Contains(id2)).ToBeTrue();
	});
});
return true;
}();
}    // namespace
