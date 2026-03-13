// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <bandit/bandit.h>
#include <PipeECS.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


struct TypeA
{};
struct TypeB
{};


go_bandit([]() {
	describe("ECS.Access", []() {
		describe("Templated", []() {
			it("Can cache pools", [&]() {
				IdContext ctx;
				TIdScope<Writes<TypeA, TypeB>> access{ctx};

				AssertThat(access.GetPool<TypeA>(), Equals(ctx.GetPool<TypeA>()));
				AssertThat(access.GetPool<const TypeA>(), Equals(ctx.GetPool<TypeA>()));
				AssertThat(access.GetPool<const TypeB>(), Equals(ctx.GetPool<TypeB>()));
			});

			it("Can check if contained", [&]() {
				IdContext ctx;
				TPool<TypeA>& pool = ctx.AssurePool<TypeA>();
				TIdScope<Writes<TypeA>> access{ctx};
				TIdScope<TypeA> accessConst{ctx};
				Id id = NoId;
				AssertThat(access.Has<TypeA>(id), Is().False());
				AssertThat(accessConst.Has<TypeA>(id), Is().False());

				id = AddId(ctx);
				AssertThat(access.Has<TypeA>(id), Is().False());
				AssertThat(accessConst.Has<TypeA>(id), Is().False());

				ctx.Add<TypeA>(id);
				AssertThat(access.Has<TypeA>(id), Is().True());
				AssertThat(accessConst.Has<TypeA>(id), Is().True());

				TIdScope<TypeA, TypeB> access2{ctx};
				ctx.Add<TypeB>(id);
				AssertThat(access2.Has<TypeB>(id), Is().True());
			});

			it("Can initialize superset", [&]() {
				IdContext ctx;
				TPool<TypeA>& typePool = ctx.AssurePool<TypeA>();

				TIdScope<Writes<TypeA, TypeB>> access1{ctx};
				TIdScope<Writes<TypeA>> superset1{access1};
				AssertThat(superset1.GetPool<TypeA>(), Equals(&typePool));

				TIdScope<Writes<TypeA, TypeB>> access2{ctx};
				TIdScope<TypeA> superset2{access2};
				AssertThat(superset2.GetPool<const TypeA>(), Equals(&typePool));

				TIdScope<Writes<TypeA, TypeB>> access3{ctx};
				TIdScope<TypeA> superset3{access3};
				AssertThat(superset1.GetPool<TypeA>(), Equals(&typePool));
			});
		});
	});
});
