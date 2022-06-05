// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <ECS/Context.h>


using namespace snowhouse;
using namespace bandit;
using namespace pipe;
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


struct EmptyComponent
{};

struct NonEmptyComponent
{
	i32 a = 0;
};


go_bandit([]() {
	describe("ECS.Components", []() {
		it("Can add one component", [&]() {
			ecs::Context ctx;
			ecs::Id id = ctx.Create();
			AssertThat(ctx.Has<EmptyComponent>(id), Is().False());
			AssertThat(ctx.TryGet<EmptyComponent>(id), Equals(nullptr));
			AssertThat(ctx.TryGet<NonEmptyComponent>(id), Equals(nullptr));

			ctx.Add<EmptyComponent>(id);
			AssertThat(ctx.Has<EmptyComponent>(id), Is().True());
			AssertThat(ctx.TryGet<EmptyComponent>(id), Equals(nullptr));

			ctx.Add<NonEmptyComponent>(id);
			AssertThat(ctx.Has<NonEmptyComponent>(id), Is().True());
			AssertThat(ctx.TryGet<NonEmptyComponent>(id), !Equals(nullptr));
		});

		it("Can remove one component", [&]() {
			ecs::Context ctx;
			ecs::Id id = ctx.Create();
			ctx.Add<EmptyComponent, NonEmptyComponent>(id);

			ctx.Remove<EmptyComponent>(id);
			AssertThat(ctx.Has<EmptyComponent>(id), Is().False());
			AssertThat(ctx.TryGet<EmptyComponent>(id), Equals(nullptr));

			ctx.Remove<NonEmptyComponent>(id);
			AssertThat(ctx.Has<NonEmptyComponent>(id), Is().False());
			AssertThat(ctx.TryGet<NonEmptyComponent>(id), Equals(nullptr));
		});

		it("Can add many components", [&]() {
			ecs::Context ctx;
			TArray<ecs::Id> ids{3};
			ctx.Create(ids);
			ctx.Add<NonEmptyComponent>(ids, {2});

			for (ecs::Id id : ids)
			{
				auto* data = ctx.TryGet<NonEmptyComponent>(id);
				AssertThat(data, !Equals(nullptr));
				AssertThat(data->a, Equals(2));
			}
		});
		it("Can remove many components", [&]() {
			ecs::Context ctx;
			TArray<ecs::Id> ids{3};
			ctx.Create(ids);
			ctx.Add<NonEmptyComponent>(ids, {2});

			TSpan<ecs::Id> firstTwo{ids.Data(), ids.Data() + 2};
			ctx.Remove<NonEmptyComponent>(firstTwo);

			AssertThat(ctx.TryGet<NonEmptyComponent>(ids[0]), Equals(nullptr));
			AssertThat(ctx.TryGet<NonEmptyComponent>(ids[1]), Equals(nullptr));
			AssertThat(ctx.TryGet<NonEmptyComponent>(ids[2]), !Equals(nullptr));
		});

		it("Components are removed after node is deleted", [&]() {
			ecs::Context ctx;
			ecs::Id id = ctx.Create();
			ctx.Add<EmptyComponent, NonEmptyComponent>(id);

			ctx.Destroy(id);
			AssertThat(ctx.IsValid(id), Is().False());

			AssertThat(ctx.Has<EmptyComponent>(id), Is().False());
			AssertThat(ctx.TryGet<EmptyComponent>(id), Equals(nullptr));
			AssertThat(ctx.Has<NonEmptyComponent>(id), Is().False());
			AssertThat(ctx.TryGet<NonEmptyComponent>(id), Equals(nullptr));
		});

		it("Components keep state when added", [&]() {
			ecs::Context ctx;
			ecs::Id id = ctx.Create();
			ctx.Add<NonEmptyComponent>(id, {2});
			AssertThat(ctx.TryGet<NonEmptyComponent>(id), !Equals(nullptr));
			AssertThat(ctx.Get<NonEmptyComponent>(id).a, Equals(2));
		});

		it("Can copy registry", []() {
			ecs::Context ctxa;

			ecs::Id id = ctxa.Create();
			ctxa.Add<EmptyComponent, NonEmptyComponent>(id);
			ecs::Id id2 = ctxa.Create();
			ctxa.Add<NonEmptyComponent>(id2, {2});

			ecs::Context ctxb{ctxa};
			AssertThat(ctxb.Has<EmptyComponent>(id), Is().True());
			AssertThat(ctxb.Has<NonEmptyComponent>(id), Is().True());
			AssertThat(ctxb.TryGet<NonEmptyComponent>(id), !Equals(nullptr));

			// Holds component values
			AssertThat(ctxb.Get<NonEmptyComponent>(id2).a, Equals(2));
		});

		it("Can check components", [&]() {
			ecs::Context ctx;
			ecs::Id id = ecs::NoId;
			AssertThat(ctx.Has<EmptyComponent>(id), Is().False());
			AssertThat(ctx.Has<NonEmptyComponent>(id), Is().False());

			id = ctx.Create();
			AssertThat(ctx.Has<EmptyComponent>(id), Is().False());
			AssertThat(ctx.Has<NonEmptyComponent>(id), Is().False());

			ctx.Add<EmptyComponent, NonEmptyComponent>(id);
			AssertThat(ctx.Has<EmptyComponent>(id), Is().True());
			AssertThat(ctx.Has<NonEmptyComponent>(id), Is().True());
		});
	});
});
