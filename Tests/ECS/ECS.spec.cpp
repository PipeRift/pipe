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
			static IdContext* ctxPtr = nullptr;

			IdContext origin;
			Id id = AddId(origin);

			ctxPtr = &origin;
			origin.Add<ATypeA>(id);

			IdContext target{origin};
			AssertThat(origin.IsValid(id), Equals(true));
			AssertThat(origin.Has<ATypeA>(id), Equals(true));
			AssertThat(target.IsValid(id), Equals(true));
			AssertThat(target.Has<ATypeA>(id), Equals(true));

			ctxPtr = &target;
			target.Add<ATypeB>(id);
			AssertThat(target.Has<ATypeB>(id), Equals(true));
		});

		it("Can move tree", [&]() {
			static IdContext* ctxPtr = nullptr;
			static bool calledAdd;

			IdContext origin;
			Id id = AddId(origin);

			calledAdd = false;
			ctxPtr    = &origin;
			origin.Add<ATypeA>(id);
			AssertThat(origin.Has<ATypeA>(id), Equals(true));
			AssertThat(calledAdd, Equals(true));

			IdContext target{Move(origin)};
			AssertThat(origin.IsValid(id), Equals(false));

			AssertThat(target.IsValid(id), Equals(true));
			AssertThat(target.Has<ATypeA>(id), Equals(true));

			calledAdd = false;
			ctxPtr    = &target;
			target.Add<ATypeB>(id);
			AssertThat(target.Has<ATypeB>(id), Equals(true));
		});

		it("Can assure pool", [&]() {
			IdContext origin;
			TPool<ATypeA>& pool = origin.AssurePool<ATypeA>();
			AssertThat(pool.Size(), Equals(0));
		});
	});
});
