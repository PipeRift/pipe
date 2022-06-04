// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <ECS/Context.h>


using namespace snowhouse;
using namespace bandit;
using namespace Pipe;
using namespace std::chrono_literals;

namespace snowhouse
{
	template<>
	struct Stringizer<ECS::Id>
	{
		static std::string ToString(ECS::Id id)
		{
			std::stringstream stream;
			stream << "Id(" << UnderlyingType<ECS::Id>(id) << ")";
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
			ECS::Context ctx;
			ECS::Id id = ctx.Create();
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
			ECS::Context ctx;
			ECS::Id id = ctx.Create();
			ctx.Add<EmptyComponent, NonEmptyComponent>(id);

			ctx.Remove<EmptyComponent>(id);
			AssertThat(ctx.Has<EmptyComponent>(id), Is().False());
			AssertThat(ctx.TryGet<EmptyComponent>(id), Equals(nullptr));

			ctx.Remove<NonEmptyComponent>(id);
			AssertThat(ctx.Has<NonEmptyComponent>(id), Is().False());
			AssertThat(ctx.TryGet<NonEmptyComponent>(id), Equals(nullptr));
		});

		it("Can add many components", [&]() {
			ECS::Context ctx;
			TArray<ECS::Id> ids{3};
			ctx.Create(ids);
			ctx.Add<NonEmptyComponent>(ids, {2});

			for (ECS::Id id : ids)
			{
				auto* data = ctx.TryGet<NonEmptyComponent>(id);
				AssertThat(data, !Equals(nullptr));
				AssertThat(data->a, Equals(2));
			}
		});
		it("Can remove many components", [&]() {
			ECS::Context ctx;
			TArray<ECS::Id> ids{3};
			ctx.Create(ids);
			ctx.Add<NonEmptyComponent>(ids, {2});

			TSpan<ECS::Id> firstTwo{ids.Data(), ids.Data() + 2};
			ctx.Remove<NonEmptyComponent>(firstTwo);

			AssertThat(ctx.TryGet<NonEmptyComponent>(ids[0]), Equals(nullptr));
			AssertThat(ctx.TryGet<NonEmptyComponent>(ids[1]), Equals(nullptr));
			AssertThat(ctx.TryGet<NonEmptyComponent>(ids[2]), !Equals(nullptr));
		});

		it("Components are removed after node is deleted", [&]() {
			ECS::Context ctx;
			ECS::Id id = ctx.Create();
			ctx.Add<EmptyComponent, NonEmptyComponent>(id);

			ctx.Destroy(id);
			AssertThat(ctx.IsValid(id), Is().False());

			AssertThat(ctx.Has<EmptyComponent>(id), Is().False());
			AssertThat(ctx.TryGet<EmptyComponent>(id), Equals(nullptr));
			AssertThat(ctx.Has<NonEmptyComponent>(id), Is().False());
			AssertThat(ctx.TryGet<NonEmptyComponent>(id), Equals(nullptr));
		});

		it("Components keep state when added", [&]() {
			ECS::Context ctx;
			ECS::Id id = ctx.Create();
			ctx.Add<NonEmptyComponent>(id, {2});
			AssertThat(ctx.TryGet<NonEmptyComponent>(id), !Equals(nullptr));
			AssertThat(ctx.Get<NonEmptyComponent>(id).a, Equals(2));
		});

		it("Can copy registry", []() {
			ECS::Context ctxa;

			ECS::Id id = ctxa.Create();
			ctxa.Add<EmptyComponent, NonEmptyComponent>(id);
			ECS::Id id2 = ctxa.Create();
			ctxa.Add<NonEmptyComponent>(id2, {2});

			ECS::Context ctxb{ctxa};
			AssertThat(ctxb.Has<EmptyComponent>(id), Is().True());
			AssertThat(ctxb.Has<NonEmptyComponent>(id), Is().True());
			AssertThat(ctxb.TryGet<NonEmptyComponent>(id), !Equals(nullptr));

			// Holds component values
			AssertThat(ctxb.Get<NonEmptyComponent>(id2).a, Equals(2));
		});

		it("Can check components", [&]() {
			ECS::Context ctx;
			ECS::Id id = ECS::NoId;
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
