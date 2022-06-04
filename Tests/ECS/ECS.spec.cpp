// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <ECS/Context.h>


using namespace snowhouse;
using namespace bandit;
using namespace Pipe;


struct Type
{
	bool value = false;
};
struct TypeTwo
{};


go_bandit([]() {
	describe("ECS", []() {
		it("Can copy tree", [&]() {
			ECS::Context origin;
			ECS::Id id = origin.Create();

			bool calledAdd = false;
			origin.OnAdd<Type>().Bind([&calledAdd, &origin](ECS::Context& ast, auto ids) {
				AssertThat(&origin, Equals(&ast));
				calledAdd = true;
			});
			origin.Add<Type>(id);
			AssertThat(calledAdd, Equals(true));

			ECS::Context target{origin};
			AssertThat(origin.IsValid(id), Equals(true));
			AssertThat(origin.Has<Type>(id), Equals(true));

			AssertThat(target.IsValid(id), Equals(true));
			AssertThat(target.Has<Type>(id), Equals(true));

			calledAdd = false;
			target.OnAdd<TypeTwo>().Bind([&calledAdd, &target](ECS::Context& ast, auto ids) {
				AssertThat(&target, Equals(&ast));
				calledAdd = true;
			});
			target.Add<TypeTwo>(id);
			AssertThat(calledAdd, Equals(true));
		});

		it("Can move tree", [&]() {
			ECS::Context origin;
			ECS::Id id = origin.Create();

			bool calledAdd = false;
			origin.OnAdd<Type>().Bind([&calledAdd, &origin](ECS::Context& ast, auto ids) {
				AssertThat(&origin, Equals(&ast));
				calledAdd = true;
			});
			origin.Add<Type>(id);
			AssertThat(calledAdd, Equals(true));

			ECS::Context target{Move(origin)};
			AssertThat(origin.IsValid(id), Equals(false));

			AssertThat(target.IsValid(id), Equals(true));
			AssertThat(target.Has<Type>(id), Equals(true));

			calledAdd = false;
			target.OnAdd<TypeTwo>().Bind([&calledAdd, &target](ECS::Context& ast, auto ids) {
				AssertThat(&target, Equals(&ast));
				calledAdd = true;
			});
			target.Add<TypeTwo>(id);
			AssertThat(calledAdd, Equals(true));
		});

		it("Can assure pool", [&]() {
			ECS::Context origin;
			ECS::TPool<Type>& pool = origin.AssurePool<Type>();
			AssertThat(&origin, Equals(&pool.GetContext()));
		});
	});
});
