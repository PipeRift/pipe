// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTests.h>
#include <PipeECS.h>


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


void RegisterECSIdScopesTests()
{
	Spec("ECS.IdScopes", []()
	{
		Describe("Templated", []()
		{
			It("Can cache pools", []()
			{
				IdContext ctx;
				TIdScope<Writes<ScopeTypeA, ScopeTypeB>> scope{ctx};

				Expect(scope.GetPool<ScopeTypeA>()).ToEqual(ctx.GetPool<ScopeTypeA>());
				Expect(scope.GetPool<const ScopeTypeA>()).ToEqual(ctx.GetPool<ScopeTypeA>());
				Expect(scope.GetPool<const ScopeTypeB>()).ToEqual(ctx.GetPool<ScopeTypeB>());
			});

			It("Can check if contained", []()
			{
				IdContext ctx;
				TPool<ScopeTypeA>& pool = ctx.AssurePool<ScopeTypeA>();
				TIdScope<Writes<ScopeTypeA>> scope{ctx};
				TIdScope<ScopeTypeA> scopeConst{ctx};
				Id id = NoId;
				Expect(scope.Has<ScopeTypeA>(id)).ToBeFalse();
				Expect(scopeConst.Has<ScopeTypeA>(id)).ToBeFalse();

				id = AddId(ctx);
				Expect(scope.Has<ScopeTypeA>(id)).ToBeFalse();
				Expect(scopeConst.Has<ScopeTypeA>(id)).ToBeFalse();

				ctx.Add<ScopeTypeA>(id);
				Expect(scope.Has<ScopeTypeA>(id)).ToBeTrue();
				Expect(scopeConst.Has<ScopeTypeA>(id)).ToBeTrue();

				TIdScope<ScopeTypeA, ScopeTypeB> scope2{ctx};
				ctx.Add<ScopeTypeB>(id);
				Expect(scope2.Has<ScopeTypeB>(id)).ToBeTrue();
			});

			It("Can initialize superset", []()
			{
				IdContext ctx;
				TPool<ScopeTypeA>& typePool = ctx.AssurePool<ScopeTypeA>();

				TIdScope<Writes<ScopeTypeA, ScopeTypeB>> scope1{ctx};
				TIdScope<Writes<ScopeTypeA>> superset1{scope1};
				Expect(superset1.GetPool<ScopeTypeA>()).ToEqual(&typePool);

				TIdScope<Writes<ScopeTypeA, ScopeTypeB>> scope2{ctx};
				TIdScope<ScopeTypeA> superset2{scope2};
				Expect(superset2.GetPool<const ScopeTypeA>()).ToEqual(&typePool);

				TIdScope<Writes<ScopeTypeA, ScopeTypeB>> scope3{ctx};
				TIdScope<ScopeTypeA> superset3{scope3};
				Expect(superset1.GetPool<ScopeTypeA>()).ToEqual(&typePool);
			});

			It("Can mark modify", []()
			{
				IdContext ctx;
				Id id = AddId(ctx);
				TIdScope<Writes<CMdfd<ScopeTypeC>>> scope1{ctx};
				Expect(scope1.Has<CMdfd<ScopeTypeC>>(id)).ToBeFalse();
				scope1.Modify<ScopeTypeC>(id);
				Expect(scope1.Has<CMdfd<ScopeTypeC>>(id)).ToBeTrue();
				Expect(scope1.IsModified<ScopeTypeC>(id)).ToBeTrue();

				scope1.Remove<CMdfd<ScopeTypeC>>(id);
				Expect(scope1.Has<CMdfd<ScopeTypeC>>(id)).ToBeFalse();
				Expect(scope1.IsModified<ScopeTypeC>(id)).ToBeFalse();

				scope1.Modify<ScopeTypeC>(id);
				Expect(scope1.Has<CMdfd<ScopeTypeC>>(id)).ToBeTrue();
				Expect(scope1.IsModified<ScopeTypeC>(id)).ToBeTrue();
			});

			It("Can mark modify automatically", []()
			{
				IdContext ctx;
				Id id         = AddId(ctx);
				using MyScope = TIdScope<Writes<ScopeTypeC, ScopeTypeB>, CMdfd<ScopeTypeB>>;
				MyScope scope{ctx};
				Expect(MyScope::WDependencies::Contains<CMdfd<ScopeTypeC>>()).ToBeTrue();
				Expect(MyScope::WDependencies::Contains<CMdfd<ScopeTypeB>>()).ToBeFalse();
				Expect(MyScope::RWDependencies::Contains<CMdfd<ScopeTypeC>>()).ToBeTrue();
				Expect(MyScope::RWDependencies::Contains<CMdfd<ScopeTypeB>>()).ToBeTrue();

				scope.Add<ScopeTypeC>(id);    // Type B should be auto modified
				Expect(scope.IsModified<ScopeTypeC>(id)).ToBeTrue();
				scope.Add<ScopeTypeB>(id);    // Type B should not be auto modified
				Expect(scope.IsModified<ScopeTypeB>(id)).ToBeFalse();

				scope.ClearPool<CMdfd<ScopeTypeC>>();
				Expect(scope.IsModified<ScopeTypeC>(id)).ToBeFalse();

				scope.Has<ScopeTypeC>(id);    // Has should never mark modify
				Expect(scope.IsModified<ScopeTypeC>(id)).ToBeFalse();

				scope.Get<const ScopeTypeC>(id);
				Expect(scope.IsModified<ScopeTypeC>(id)).ToBeFalse();
				scope.Get<ScopeTypeC>(id);
				Expect(scope.IsModified<ScopeTypeC>(id)).ToBeTrue();
				scope.Add<ScopeTypeB>(id);    // Type B should not be auto modified
				Expect(scope.IsModified<ScopeTypeB>(id)).ToBeFalse();

				scope.ClearPool<CMdfd<ScopeTypeC>>();

				scope.Remove<ScopeTypeC>(id);
				Expect(scope.Has<ScopeTypeC>(id)).ToBeFalse();
				Expect(scope.IsModified<ScopeTypeC>(id)).ToBeTrue();

				scope.Remove<ScopeTypeB>(id);    // Type B should not be auto modified
				Expect(scope.Has<ScopeTypeB>(id)).ToBeFalse();
				Expect(scope.IsModified<ScopeTypeB>(id)).ToBeFalse();
			});
		});
	});
}
