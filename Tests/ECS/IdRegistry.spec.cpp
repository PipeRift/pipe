// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <ECS/IdRegistry.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;
using namespace std::chrono_literals;

namespace snowhouse
{
	template<>
	struct Stringizer<ecs::Id>
	{
		static std::string ToString(ecs::Id id)
		{
			std::stringstream stream;
			stream << "Id(" << UnderlyingType<ecs::Id>(id) << ")";
			return stream.str();
		}
	};
}    // namespace snowhouse


go_bandit([]() {
	describe("ECS.IdRegistry", []() {
		it("Can create one id", [&]() {
			ecs::IdRegistry ids;
			AssertThat(ids.Size(), Equals(0));

			ecs::Id id = ids.Create();
			AssertThat(id, !Equals(ecs::Id(ecs::NoId)));
			AssertThat(ids.IsValid(id), Is().True());
			AssertThat(ids.Size(), Equals(1));
		});

		it("Can remove one id", [&]() {
			ecs::IdRegistry ids;
			ecs::Id id = ids.Create();
			AssertThat(ids.Size(), Equals(1));

			AssertThat(ids.Destroy(id), Is().True());
			AssertThat(ids.IsValid(id), Is().False());
			AssertThat(ids.Size(), Equals(0));
		});

		it("Can create two and remove first", [&]() {
			ecs::IdRegistry ids;

			ecs::Id id1 = ids.Create();
			ecs::Id id2 = ids.Create();
			AssertThat(ids.Destroy(id1), Is().True());
			AssertThat(ids.IsValid(id1), Is().False());
			AssertThat(ids.Size(), Equals(1));
		});

		it("Can create two and remove last", [&]() {
			ecs::IdRegistry ids;

			ecs::Id id1 = ids.Create();
			ecs::Id id2 = ids.Create();

			AssertThat(ids.Destroy(id2), Is().True());
			AssertThat(ids.IsValid(id2), Is().False());
			AssertThat(ids.Size(), Equals(1));
		});

		it("Can create many ids", [&]() {
			ecs::IdRegistry ids;
			AssertThat(ids.Size(), Equals(0));

			TArray<ecs::Id> list(3);
			ids.Create(list);

			AssertThat(ids.Size(), Equals(3));
			for (i32 i = 0; i < list.Size(); ++i)
			{
				AssertThat(list[i], Equals(ecs::Id(i)));
				AssertThat(ids.IsValid(list[i]), Is().True());
			}
		});

		it("Can remove many ids", [&]() {
			ecs::IdRegistry ids;
			TArray<ecs::Id> list(3);
			ids.Create(list);
			AssertThat(ids.Size(), Equals(3));

			AssertThat(ids.Destroy(list), Is().True());
			AssertThat(ids.Size(), Equals(0));

			for (i32 i = 0; i < list.Size(); ++i)
			{
				AssertThat(ids.IsValid(list[i]), Is().False());
			}
		});
	});
});
