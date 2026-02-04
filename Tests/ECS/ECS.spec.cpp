// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <bandit/bandit.h>
#include <PipeECS.h>


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
			static EntityContext* astPtr = nullptr;
			static bool calledAdd;

			EntityContext origin;
			Id id = AddId(origin);

			calledAdd = false;
			astPtr    = &origin;
			origin.OnAdd<ATypeA>().Bind([](EntityContext& ast, auto ids) {
				for (Id id : ids)
				{
					AssertThat(ast.Has<ATypeA>(id), Equals(true));
				}
				AssertThat(astPtr, Equals(&ast));
				calledAdd = true;
			});
			origin.Add<ATypeA>(id);
			AssertThat(calledAdd, Equals(true));

			EntityContext target{origin};
			AssertThat(origin.IsValid(id), Equals(true));
			AssertThat(origin.Has<ATypeA>(id), Equals(true));

			AssertThat(target.IsValid(id), Equals(true));
			AssertThat(target.Has<ATypeA>(id), Equals(true));

			calledAdd = false;
			astPtr    = &target;
			target.OnAdd<ATypeB>().Bind([](EntityContext& ast, auto ids) {
				for (Id id : ids)
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
			static EntityContext* astPtr = nullptr;
			static bool calledAdd;

			EntityContext origin;
			Id id = AddId(origin);

			calledAdd = false;
			astPtr    = &origin;
			origin.OnAdd<ATypeA>().Bind([](EntityContext& ast, auto ids) {
				for (Id id : ids)
				{
					AssertThat(ast.Has<ATypeA>(id), Equals(true));
				}
				AssertThat(astPtr, Equals(&ast));
				calledAdd = true;
			});
			origin.Add<ATypeA>(id);
			AssertThat(calledAdd, Equals(true));

			EntityContext target{Move(origin)};
			AssertThat(origin.IsValid(id), Equals(false));

			AssertThat(target.IsValid(id), Equals(true));
			AssertThat(target.Has<ATypeA>(id), Equals(true));

			calledAdd = false;
			astPtr    = &target;
			target.OnAdd<ATypeB>().Bind([](EntityContext& ast, auto ids) {
				for (Id id : ids)
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
			EntityContext origin;
			TPool<ATypeA>& pool = origin.AssurePool<ATypeA>();
			AssertThat(&origin, Equals(&pool.GetContext()));
		});
	});
});
