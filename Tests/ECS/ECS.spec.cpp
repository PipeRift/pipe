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
			static EntityContext* ctxPtr = nullptr;
			static bool calledAdd;

			EntityContext origin;
			Id id = AddId(origin);

			calledAdd = false;
			ctxPtr    = &origin;
			origin.OnAdd<ATypeA>().Bind([&origin](auto ids) {
				for (Id id : ids)
				{
					AssertThat(origin.Has<ATypeA>(id), Equals(true));
				}
				AssertThat(ctxPtr, Equals(&origin));
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
			ctxPtr    = &target;
			target.OnAdd<ATypeB>().Bind([&target](auto ids) {
				for (Id id : ids)
				{
					AssertThat(target.Has<ATypeB>(id), Equals(true));
				}
				AssertThat(ctxPtr, Equals(&target));
				calledAdd = true;
			});
			target.Add<ATypeB>(id);
			AssertThat(calledAdd, Equals(true));
		});

		it("Can move tree", [&]() {
			static EntityContext* ctxPtr = nullptr;
			static bool calledAdd;

			EntityContext origin;
			Id id = AddId(origin);

			calledAdd = false;
			ctxPtr    = &origin;
			origin.OnAdd<ATypeA>().Bind([&origin](auto ids) {
				for (Id id : ids)
				{
					AssertThat(origin.Has<ATypeA>(id), Equals(true));
				}
				AssertThat(ctxPtr, Equals(&origin));
				calledAdd = true;
			});
			origin.Add<ATypeA>(id);
			AssertThat(calledAdd, Equals(true));

			EntityContext target{Move(origin)};
			AssertThat(origin.IsValid(id), Equals(false));

			AssertThat(target.IsValid(id), Equals(true));
			AssertThat(target.Has<ATypeA>(id), Equals(true));

			calledAdd = false;
			ctxPtr    = &target;
			target.OnAdd<ATypeB>().Bind([&target](auto ids) {
				for (Id id : ids)
				{
					AssertThat(target.Has<ATypeB>(id), Equals(true));
				}
				AssertThat(ctxPtr, Equals(&target));
				calledAdd = true;
			});
			target.Add<ATypeB>(id);
			AssertThat(calledAdd, Equals(true));
		});

		it("Can assure pool", [&]() {
			EntityContext origin;
			TPool<ATypeA>& pool = origin.AssurePool<ATypeA>();
			AssertThat(pool.Size(), Equals(0));
		});
	});
});
