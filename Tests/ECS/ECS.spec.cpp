// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <ECS/Context.h>


using namespace snowhouse;
using namespace bandit;
using namespace pipe;


struct TypeA
{
	bool value = false;
};
struct TypeB
{};


go_bandit([]() {
	describe("ECS", []() {
		it("Can copy tree", [&]() {
			ecs::Context origin;
			ecs::Id id = origin.Create();

			bool calledAdd = false;
			origin.OnAdd<TypeA>().Bind([&calledAdd, &origin](ecs::Context& ast, auto ids) {
				AssertThat(&origin, Equals(&ast));
				calledAdd = true;
			});
			origin.Add<TypeA>(id);
			AssertThat(calledAdd, Equals(true));

			ecs::Context target{origin};
			AssertThat(origin.IsValid(id), Equals(true));
			AssertThat(origin.Has<TypeA>(id), Equals(true));

			AssertThat(target.IsValid(id), Equals(true));
			AssertThat(target.Has<TypeA>(id), Equals(true));

			calledAdd = false;
			target.OnAdd<TypeB>().Bind([&calledAdd, &target](ecs::Context& ast, auto ids) {
				AssertThat(&target, Equals(&ast));
				calledAdd = true;
			});
			target.Add<TypeB>(id);
			AssertThat(calledAdd, Equals(true));
		});

		it("Can move tree", [&]() {
			ecs::Context origin;
			ecs::Id id = origin.Create();

			bool calledAdd = false;
			origin.OnAdd<TypeA>().Bind([&calledAdd, &origin](ecs::Context& ast, auto ids) {
				AssertThat(&origin, Equals(&ast));
				calledAdd = true;
			});
			origin.Add<TypeA>(id);
			AssertThat(calledAdd, Equals(true));

			ecs::Context target{Move(origin)};
			AssertThat(origin.IsValid(id), Equals(false));

			AssertThat(target.IsValid(id), Equals(true));
			AssertThat(target.Has<TypeA>(id), Equals(true));

			calledAdd = false;
			target.OnAdd<TypeB>().Bind([&calledAdd, &target](ecs::Context& ast, auto ids) {
				AssertThat(&target, Equals(&ast));
				calledAdd = true;
			});
			target.Add<TypeB>(id);
			AssertThat(calledAdd, Equals(true));
		});

		it("Can assure pool", [&]() {
			ecs::Context origin;
			ecs::TPool<TypeA>& pool = origin.AssurePool<TypeA>();
			AssertThat(&origin, Equals(&pool.GetContext()));
		});
	});
});
