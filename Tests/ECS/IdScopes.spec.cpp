// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <bandit/bandit.h>
#include <PipeECS.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


struct TypeA
{};
struct TypeB
{
	bool data;    // Not empty type
};
struct TypeC
{
	P_STRUCT(TypeC, TF_ECS_ModifyOnEdit)

	bool data;    // Not empty type
};


go_bandit([]() {
	describe("ECS.IdScopes", []() {
		describe("Templated", []() {
			it("Can cache pools", [&]() {
				IdContext ctx;
				TIdScope<Writes<TypeA, TypeB>> scope{ctx};

				AssertThat(scope.GetPool<TypeA>(), Equals(ctx.GetPool<TypeA>()));
				AssertThat(scope.GetPool<const TypeA>(), Equals(ctx.GetPool<TypeA>()));
				AssertThat(scope.GetPool<const TypeB>(), Equals(ctx.GetPool<TypeB>()));
			});

			it("Can check if contained", [&]() {
				IdContext ctx;
				TPool<TypeA>& pool = ctx.AssurePool<TypeA>();
				TIdScope<Writes<TypeA>> scope{ctx};
				TIdScope<TypeA> scopeConst{ctx};
				Id id = NoId;
				AssertThat(scope.Has<TypeA>(id), Is().False());
				AssertThat(scopeConst.Has<TypeA>(id), Is().False());

				id = AddId(ctx);
				AssertThat(scope.Has<TypeA>(id), Is().False());
				AssertThat(scopeConst.Has<TypeA>(id), Is().False());

				ctx.Add<TypeA>(id);
				AssertThat(scope.Has<TypeA>(id), Is().True());
				AssertThat(scopeConst.Has<TypeA>(id), Is().True());

				TIdScope<TypeA, TypeB> scope2{ctx};
				ctx.Add<TypeB>(id);
				AssertThat(scope2.Has<TypeB>(id), Is().True());
			});

			it("Can initialize superset", [&]() {
				IdContext ctx;
				TPool<TypeA>& typePool = ctx.AssurePool<TypeA>();

				TIdScope<Writes<TypeA, TypeB>> scope1{ctx};
				TIdScope<Writes<TypeA>> superset1{scope1};
				AssertThat(superset1.GetPool<TypeA>(), Equals(&typePool));

				TIdScope<Writes<TypeA, TypeB>> scope2{ctx};
				TIdScope<TypeA> superset2{scope2};
				AssertThat(superset2.GetPool<const TypeA>(), Equals(&typePool));

				TIdScope<Writes<TypeA, TypeB>> scope3{ctx};
				TIdScope<TypeA> superset3{scope3};
				AssertThat(superset1.GetPool<TypeA>(), Equals(&typePool));
			});

			it("Can mark modify", [&]() {
				IdContext ctx;
				Id id = AddId(ctx);
				TIdScope<Writes<CMdfd<TypeC>>> scope1{ctx};
				AssertThat(scope1.Has<CMdfd<TypeC>>(id), Is().False());
				scope1.Modify<TypeC>(id);
				AssertThat(scope1.Has<CMdfd<TypeC>>(id), Is().True());
				AssertThat(scope1.IsModified<TypeC>(id), Is().True());

				scope1.Remove<CMdfd<TypeC>>(id);
				AssertThat(scope1.Has<CMdfd<TypeC>>(id), Is().False());
				AssertThat(scope1.IsModified<TypeC>(id), Is().False());

				scope1.Modify<TypeC>(id);
				AssertThat(scope1.Has<CMdfd<TypeC>>(id), Is().True());
				AssertThat(scope1.IsModified<TypeC>(id), Is().True());
			});

			it("Can mark modify automatically", [&]() {
				IdContext ctx;
				Id id         = AddId(ctx);
				using MyScope = TIdScope<Writes<TypeC, TypeB>, CMdfd<TypeB>>;
				MyScope scope{ctx};
				AssertThat(MyScope::WDependencies::Contains<CMdfd<TypeC>>(), Is().True());
				AssertThat(MyScope::WDependencies::Contains<CMdfd<TypeB>>(), Is().False());
				AssertThat(MyScope::RWDependencies::Contains<CMdfd<TypeC>>(), Is().True());
				AssertThat(MyScope::RWDependencies::Contains<CMdfd<TypeB>>(), Is().True());

				scope.Add<TypeC>(id);    // Type B should be auto modified
				AssertThat(scope.IsModified<TypeC>(id), Is().True());
				scope.Add<TypeB>(id);    // Type B should not be auto modified
				AssertThat(scope.IsModified<TypeB>(id), Is().False());

				scope.ClearPool<CMdfd<TypeC>>();
				AssertThat(scope.IsModified<TypeC>(id), Is().False());

				scope.Has<TypeC>(id);    // Has should never mark modify
				AssertThat(scope.IsModified<TypeC>(id), Is().False());

				scope.Get<const TypeC>(id);
				AssertThat(scope.IsModified<TypeC>(id), Is().False());
				scope.Get<TypeC>(id);
				AssertThat(scope.IsModified<TypeC>(id), Is().True());
				scope.Add<TypeB>(id);    // Type B should not be auto modified
				AssertThat(scope.IsModified<TypeB>(id), Is().False());

				scope.ClearPool<CMdfd<TypeC>>();

				scope.Remove<TypeC>(id);
				AssertThat(scope.Has<TypeC>(id), Is().False());
				AssertThat(scope.IsModified<TypeC>(id), Is().True());

				scope.Remove<TypeB>(id);    // Type B should not be auto modified
				AssertThat(scope.Has<TypeB>(id), Is().False());
				AssertThat(scope.IsModified<TypeB>(id), Is().False());
			});
		});
	});
});
