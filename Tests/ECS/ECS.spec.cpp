// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTests.h>
#include <PipeECS.h>


using namespace p;


struct ECSTypeA
{
	bool value = false;
};
struct ECSTypeB
{};


void RegisterECSECSsmTests()
{
	Spec("ECS", []()
	{
		It("Can copy context", []()
		{
			static IdContext* ctxPtr = nullptr;

			IdContext origin;
			Id id = AddId(origin);

			ctxPtr = &origin;
			origin.Add<ECSTypeA>(id);

			IdContext target{origin};
			Expect(origin.IsValid(id)).ToEqual(true);
			Expect(origin.Has<ECSTypeA>(id)).ToEqual(true);
			Expect(target.IsValid(id)).ToEqual(true);
			Expect(target.Has<ECSTypeA>(id)).ToEqual(true);

			ctxPtr = &target;
			target.Add<ECSTypeB>(id);
			Expect(target.Has<ECSTypeB>(id)).ToEqual(true);
		});

		It("Can move context", []()
		{
			static IdContext* ctxPtr = nullptr;

			IdContext origin;
			Id id = AddId(origin);

			ctxPtr = &origin;
			origin.Add<ECSTypeA>(id);
			Expect(origin.Has<ECSTypeA>(id)).ToEqual(true);

			IdContext target{Move(origin)};
			Expect(origin.IsValid(id)).ToEqual(false);

			Expect(target.IsValid(id)).ToEqual(true);
			Expect(target.Has<ECSTypeA>(id)).ToEqual(true);

			ctxPtr = &target;
			target.Add<ECSTypeB>(id);
			Expect(target.Has<ECSTypeB>(id)).ToEqual(true);
		});

		It("Can assure pool", []()
		{
			IdContext origin;
			TPool<ECSTypeA>& pool = origin.AssurePool<ECSTypeA>();
			Expect(pool.Size()).ToEqual(0);
		});
	});
}
