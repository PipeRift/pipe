// Copyright 2015-2023 Piperift - All rights reserved

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
			static ecs::Context* astPtr = nullptr;
			static bool calledAdd;

			ecs::Context origin;
			ecs::Id id = origin.Create();

			calledAdd = false;
			astPtr    = &origin;
			origin.OnAdd<ATypeA>().Bind([](ecs::Context& ast, auto ids) {
				for (ecs::Id id : ids)
				{
					AssertThat(ast.Has<ATypeA>(id), Equals(true));
				}
				AssertThat(astPtr, Equals(&ast));
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
			astPtr    = &target;
			target.OnAdd<ATypeB>().Bind([](ecs::Context& ast, auto ids) {
				for (ecs::Id id : ids)
				{
					AssertThat(ast.Has<ATypeB>(id), Equals(true));
				}
				AssertThat(astPtr, Equals(&ast));
				calledAdd = true;
			});

			target.Add<ATypeB>(id);
			AssertThat(calledAdd, Equals(true));
		});

		it("Can move tree", [&]() {
			static ecs::Context* astPtr = nullptr;
			static bool calledAdd;

			ecs::Context origin;
			ecs::Id id = origin.Create();

			calledAdd = false;
			astPtr    = &origin;
			origin.OnAdd<ATypeA>().Bind([](ecs::Context& ast, auto ids) {
				for (ecs::Id id : ids)
				{
					AssertThat(ast.Has<ATypeA>(id), Equals(true));
				}
				AssertThat(astPtr, Equals(&ast));
				calledAdd = true;
			});
			origin.Add<ATypeA>(id);
			AssertThat(calledAdd, Equals(true));

			ecs::Context target{Move(origin)};
			AssertThat(origin.IsValid(id), Equals(false));

			AssertThat(target.IsValid(id), Equals(true));
			AssertThat(target.Has<ATypeA>(id), Equals(true));

			calledAdd = false;
			astPtr    = &target;
			target.OnAdd<ATypeB>().Bind([](ecs::Context& ast, auto ids) {
				for (ecs::Id id : ids)
				{
					AssertThat(ast.Has<ATypeB>(id), Equals(true));
				}
				AssertThat(astPtr, Equals(&ast));
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
