// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/ECS/Context.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


struct ATypeA
{
	bool value = false;
};
struct ATypeB
{};


go_bandit([]() {
	describe("ECS", []() {
		it("Can copy tree", [&]() {
			ecs::Context origin;
			ecs::Id id = origin.Create();

			bool calledAdd = false;
			origin.OnAdd<ATypeA>().Bind([&calledAdd, &origin](ecs::Context& ast, auto ids) {
				AssertThat(&origin, Equals(&ast));
				calledAdd = true;
			});
			origin.Add<ATypeA>(id);
			AssertThat(calledAdd, Equals(true));

			ecs::Context target{origin};
			AssertThat(origin.IsValid(id), Equals(true));
			AssertThat(origin.Has<ATypeA>(id), Equals(true));

			AssertThat(target.IsValid(id), Equals(true));
			AssertThat(target.Has<ATypeA>(id), Equals(true));

			calledAdd = false;
			target.OnAdd<ATypeB>().Bind([&calledAdd, &target](ecs::Context& ast, auto ids) {
				AssertThat(&target, Equals(&ast));
				calledAdd = true;
			});
			target.Add<ATypeB>(id);
			AssertThat(calledAdd, Equals(true));
		});

		it("Can move tree", [&]() {
			ecs::Context origin;
			ecs::Id id = origin.Create();

			bool calledAdd = false;
			origin.OnAdd<ATypeA>().Bind([&calledAdd, &origin](ecs::Context& ast, auto ids) {
				AssertThat(&origin, Equals(&ast));
				calledAdd = true;
			});
			origin.Add<ATypeA>(id);
			AssertThat(calledAdd, Equals(true));

			ecs::Context target{Move(origin)};
			AssertThat(origin.IsValid(id), Equals(false));

			AssertThat(target.IsValid(id), Equals(true));
			AssertThat(target.Has<ATypeA>(id), Equals(true));

			calledAdd = false;
			target.OnAdd<ATypeB>().Bind([&calledAdd, &target](ecs::Context& ast, auto ids) {
				AssertThat(&target, Equals(&ast));
				calledAdd = true;
			});
			target.Add<ATypeB>(id);
			AssertThat(calledAdd, Equals(true));
		});

		it("Can assure pool", [&]() {
			ecs::Context origin;
			ecs::TPool<ATypeA>& pool = origin.AssurePool<ATypeA>();
			AssertThat(&origin, Equals(&pool.GetContext()));
		});
	});
});
