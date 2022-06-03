// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <ECS/Access.h>
#include <ECS/Context.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;


struct Type
{};
struct TypeTwo
{};
struct TypeThree
{};


go_bandit([]() {
	describe("ECS.Access", []() {
		describe("Templated", []() {
			it("Can cache pools", [&]() {
				ECS::Context ctx;
				TAccess<TWrite<Type>, TypeTwo> access{ctx};

				AssertThat(access.GetPool<Type>(), Equals(ctx.GetPool<Type>()));
				AssertThat(access.GetPool<const Type>(), Equals(ctx.GetPool<Type>()));
				AssertThat(access.GetPool<const TypeTwo>(), Equals(ctx.GetPool<TypeTwo>()));
			});

			it("Can check if contained", [&]() {
				ECS::Context ctx;
				ECS::TPool<Type>& pool = ctx.AssurePool<Type>();
				TAccess<TWrite<Type>> access{ctx};
				TAccess<Type> accessConst{ctx};
				ECS::Id id = ECS::NoId;
				AssertThat(access.Has<Type>(id), Is().False());
				AssertThat(accessConst.Has<Type>(id), Is().False());

				id = ctx.Create();
				AssertThat(access.Has<Type>(id), Is().False());
				AssertThat(accessConst.Has<Type>(id), Is().False());

				ctx.Add<Type>(id);
				AssertThat(access.Has<Type>(id), Is().True());
				AssertThat(accessConst.Has<Type>(id), Is().True());

				TAccess<Type, TypeTwo> access2{ctx};
				ctx.Add<TypeTwo>(id);
				AssertThat(access2.Has<TypeTwo>(id), Is().True());
			});

			it("Can initialize superset", [&]() {
				ECS::Context ctx;
				ECS::TPool<Type>& typePool = ctx.AssurePool<Type>();

				TAccess<TWrite<Type>, TWrite<TypeTwo>> access1{ctx};
				TAccess<TWrite<Type>> superset1{access1};
				AssertThat(superset1.GetPool<Type>(), Equals(&typePool));

				TAccess<TWrite<Type>, TWrite<TypeTwo>> access2{ctx};
				TAccess<Type> superset2{access2};
				AssertThat(superset2.GetPool<const Type>(), Equals(&typePool));

				TAccess<TWrite<Type>, TWrite<TypeTwo>> access3{ctx};
				TAccess<Type> superset3{access3};
				AssertThat(superset1.GetPool<Type>(), Equals(&typePool));
			});
		});

		xdescribe("Runtime", []() {
			xit("Can cache pools", [&]() {});
		});
	});
});
