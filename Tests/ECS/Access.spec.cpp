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
				EntityContext ctx;
				TAccess<TWrite<TypeA>, TypeB> access{ctx};

				AssertThat(access.GetPool<TypeA>(), Equals(ctx.GetPool<TypeA>()));
				AssertThat(access.GetPool<const TypeA>(), Equals(ctx.GetPool<TypeA>()));
				AssertThat(access.GetPool<const TypeB>(), Equals(ctx.GetPool<TypeB>()));
			});

			it("Can check if contained", [&]() {
				EntityContext ctx;
				TPool<TypeA>& pool = ctx.AssurePool<TypeA>();
				TAccess<TWrite<TypeA>> access{ctx};
				TAccess<TypeA> accessConst{ctx};
				Id id = NoId;
				AssertThat(access.Has<TypeA>(id), Is().False());
				AssertThat(accessConst.Has<TypeA>(id), Is().False());

				id = ctx.Create();
				AssertThat(access.Has<TypeA>(id), Is().False());
				AssertThat(accessConst.Has<TypeA>(id), Is().False());

				ctx.Add<TypeA>(id);
				AssertThat(access.Has<TypeA>(id), Is().True());
				AssertThat(accessConst.Has<TypeA>(id), Is().True());

				TAccess<TypeA, TypeB> access2{ctx};
				ctx.Add<TypeB>(id);
				AssertThat(access2.Has<TypeB>(id), Is().True());
			});

			it("Can initialize superset", [&]() {
				EntityContext ctx;
				TPool<TypeA>& typePool = ctx.AssurePool<TypeA>();

				TAccess<TWrite<TypeA>, TWrite<TypeB>> access1{ctx};
				TAccess<TWrite<TypeA>> superset1{access1};
				AssertThat(superset1.GetPool<TypeA>(), Equals(&typePool));

				TAccess<TWrite<TypeA>, TWrite<TypeB>> access2{ctx};
				TAccess<TypeA> superset2{access2};
				AssertThat(superset2.GetPool<const TypeA>(), Equals(&typePool));

				TAccess<TWrite<TypeA>, TWrite<TypeB>> access3{ctx};
				TAccess<TypeA> superset3{access3};
				AssertThat(superset1.GetPool<TypeA>(), Equals(&typePool));
			});
		});
	});
});
