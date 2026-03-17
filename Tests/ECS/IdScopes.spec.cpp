// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <bandit/bandit.h>
#include <PipeECS.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


struct ScopeTypeA
{};
struct ScopeTypeB
{
	bool data;    // Not empty type
};
struct ScopeTypeC
{
	P_STRUCT(ScopeTypeC, TF_ECS_ModifyOnEdit)

	bool data;    // Not empty type
};


go_bandit([]() {
	describe("ECS.IdScopes", []() {
		describe("Templated", []() {
			it("Can cache pools", [&]() {
				IdContext ctx;
				TIdScope<Writes<ScopeTypeA, ScopeTypeB>> scope{ctx};

				AssertThat(scope.GetPool<ScopeTypeA>(), Equals(ctx.GetPool<ScopeTypeA>()));
				AssertThat(scope.GetPool<const ScopeTypeA>(), Equals(ctx.GetPool<ScopeTypeA>()));
				AssertThat(scope.GetPool<const ScopeTypeB>(), Equals(ctx.GetPool<ScopeTypeB>()));
			});

			it("Can check if contained", [&]() {
				IdContext ctx;
				TPool<ScopeTypeA>& pool = ctx.AssurePool<ScopeTypeA>();
				TIdScope<Writes<ScopeTypeA>> scope{ctx};
				TIdScope<ScopeTypeA> scopeConst{ctx};
				Id id = NoId;
				AssertThat(scope.Has<ScopeTypeA>(id), Is().False());
				AssertThat(scopeConst.Has<ScopeTypeA>(id), Is().False());

				id = AddId(ctx);
				AssertThat(scope.Has<ScopeTypeA>(id), Is().False());
				AssertThat(scopeConst.Has<ScopeTypeA>(id), Is().False());

				ctx.Add<ScopeTypeA>(id);
				AssertThat(scope.Has<ScopeTypeA>(id), Is().True());
				AssertThat(scopeConst.Has<ScopeTypeA>(id), Is().True());

				TIdScope<ScopeTypeA, ScopeTypeB> scope2{ctx};
				ctx.Add<ScopeTypeB>(id);
				AssertThat(scope2.Has<ScopeTypeB>(id), Is().True());
			});

			it("Can initialize superset", [&]() {
				IdContext ctx;
				TPool<ScopeTypeA>& typePool = ctx.AssurePool<ScopeTypeA>();

				TIdScope<Writes<ScopeTypeA, ScopeTypeB>> scope1{ctx};
				TIdScope<Writes<ScopeTypeA>> superset1{scope1};
				AssertThat(superset1.GetPool<ScopeTypeA>(), Equals(&typePool));

				TIdScope<Writes<ScopeTypeA, ScopeTypeB>> scope2{ctx};
				TIdScope<ScopeTypeA> superset2{scope2};
				AssertThat(superset2.GetPool<const ScopeTypeA>(), Equals(&typePool));

				TIdScope<Writes<ScopeTypeA, ScopeTypeB>> scope3{ctx};
				TIdScope<ScopeTypeA> superset3{scope3};
				AssertThat(superset1.GetPool<ScopeTypeA>(), Equals(&typePool));
			});

			it("Can mark modify", [&]() {
				IdContext ctx;
				Id id = AddId(ctx);
				TIdScope<Writes<CMdfd<ScopeTypeC>>> scope1{ctx};
				AssertThat(scope1.Has<CMdfd<ScopeTypeC>>(id), Is().False());
				scope1.Modify<ScopeTypeC>(id);
				AssertThat(scope1.Has<CMdfd<ScopeTypeC>>(id), Is().True());
				AssertThat(scope1.IsModified<ScopeTypeC>(id), Is().True());

				scope1.Remove<CMdfd<ScopeTypeC>>(id);
				AssertThat(scope1.Has<CMdfd<ScopeTypeC>>(id), Is().False());
				AssertThat(scope1.IsModified<ScopeTypeC>(id), Is().False());

				scope1.Modify<ScopeTypeC>(id);
				AssertThat(scope1.Has<CMdfd<ScopeTypeC>>(id), Is().True());
				AssertThat(scope1.IsModified<ScopeTypeC>(id), Is().True());
			});

			it("Can mark modify automatically", [&]() {
				IdContext ctx;
				Id id         = AddId(ctx);
				using MyScope = TIdScope<Writes<ScopeTypeC, ScopeTypeB>, CMdfd<ScopeTypeB>>;
				MyScope scope{ctx};
				AssertThat(MyScope::WDependencies::Contains<CMdfd<ScopeTypeC>>(), Is().True());
				AssertThat(MyScope::WDependencies::Contains<CMdfd<ScopeTypeB>>(), Is().False());
				AssertThat(MyScope::RWDependencies::Contains<CMdfd<ScopeTypeC>>(), Is().True());
				AssertThat(MyScope::RWDependencies::Contains<CMdfd<ScopeTypeB>>(), Is().True());

				scope.Add<ScopeTypeC>(id);    // Type B should be auto modified
				AssertThat(scope.IsModified<ScopeTypeC>(id), Is().True());
				scope.Add<ScopeTypeB>(id);    // Type B should not be auto modified
				AssertThat(scope.IsModified<ScopeTypeB>(id), Is().False());

				scope.ClearPool<CMdfd<ScopeTypeC>>();
				AssertThat(scope.IsModified<ScopeTypeC>(id), Is().False());

				scope.Has<ScopeTypeC>(id);    // Has should never mark modify
				AssertThat(scope.IsModified<ScopeTypeC>(id), Is().False());

				scope.Get<const ScopeTypeC>(id);
				AssertThat(scope.IsModified<ScopeTypeC>(id), Is().False());
				scope.Get<ScopeTypeC>(id);
				AssertThat(scope.IsModified<ScopeTypeC>(id), Is().True());
				scope.Add<ScopeTypeB>(id);    // Type B should not be auto modified
				AssertThat(scope.IsModified<ScopeTypeB>(id), Is().False());

				scope.ClearPool<CMdfd<ScopeTypeC>>();

				scope.Remove<ScopeTypeC>(id);
				AssertThat(scope.Has<ScopeTypeC>(id), Is().False());
				AssertThat(scope.IsModified<ScopeTypeC>(id), Is().True());

				scope.Remove<ScopeTypeB>(id);    // Type B should not be auto modified
				AssertThat(scope.Has<ScopeTypeB>(id), Is().False());
				AssertThat(scope.IsModified<ScopeTypeB>(id), Is().False());
			});
		});
	});
});
