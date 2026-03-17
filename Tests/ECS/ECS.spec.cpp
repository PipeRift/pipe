// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <bandit/bandit.h>
#include <PipeECS.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


struct ECSTypeA
{
	bool value = false;
};
struct ECSTypeB
{};


go_bandit([]() {
	describe("ECS", []() {
		it("Can copy context", [&]() {
			static IdContext* ctxPtr = nullptr;

			IdContext origin;
			Id id = AddId(origin);

			ctxPtr = &origin;
			origin.Add<ECSTypeA>(id);

			IdContext target{origin};
			AssertThat(origin.IsValid(id), Equals(true));
			AssertThat(origin.Has<ECSTypeA>(id), Equals(true));
			AssertThat(target.IsValid(id), Equals(true));
			AssertThat(target.Has<ECSTypeA>(id), Equals(true));

			ctxPtr = &target;
			target.Add<ECSTypeB>(id);
			AssertThat(target.Has<ECSTypeB>(id), Equals(true));
		});

		it("Can move context", [&]() {
			static IdContext* ctxPtr = nullptr;

			IdContext origin;
			Id id = AddId(origin);

			ctxPtr = &origin;
			origin.Add<ECSTypeA>(id);
			AssertThat(origin.Has<ECSTypeA>(id), Equals(true));

			IdContext target{Move(origin)};
			AssertThat(origin.IsValid(id), Equals(false));

			AssertThat(target.IsValid(id), Equals(true));
			AssertThat(target.Has<ECSTypeA>(id), Equals(true));

			ctxPtr = &target;
			target.Add<ECSTypeB>(id);
			AssertThat(target.Has<ECSTypeB>(id), Equals(true));
		});

		it("Can assure pool", [&]() {
			IdContext origin;
			TPool<ECSTypeA>& pool = origin.AssurePool<ECSTypeA>();
			AssertThat(pool.Size(), Equals(0));
		});
	});
});
