// Copyright 2015-2023 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/PipeECS.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;
using namespace std::chrono_literals;

namespace snowhouse
{
	template<>
	struct Stringizer<Id>
	{
		static std::string ToString(Id id)
		{
			std::stringstream stream;
			stream << "Id(" << UnderlyingType<Id>(id) << ")";
			return stream.str();
		}
	};
}    // namespace snowhouse


struct EmptyComponent
{
	static u32 destructed;

	~EmptyComponent()
	{
		++destructed;
	}
};
u32 EmptyComponent::destructed = 0;

struct NonEmptyComponent
{
	static u32 destructed;
	i32 a = 0;

	~NonEmptyComponent()
	{
		++destructed;
	}
};
u32 NonEmptyComponent::destructed = 0;

struct TestComponent
{
	static u32 destructed;
	bool d;

	~TestComponent()
	{
		++destructed;
	}
};
u32 TestComponent::destructed = 0;


go_bandit([]() {
	describe("ECS.Components", []() {
		it("Can add one component", [&]() {
			EntityContext ctx;
			Id id = ctx.Create();
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
			EntityContext ctx;
			Id id = ctx.Create();
			ctx.Add<EmptyComponent, NonEmptyComponent>(id);

			ctx.Remove<EmptyComponent>(id);
			AssertThat(ctx.Has<EmptyComponent>(id), Is().False());
			AssertThat(ctx.TryGet<EmptyComponent>(id), Equals(nullptr));

			NonEmptyComponent::destructed = 0;
			ctx.Remove<NonEmptyComponent>(id);
			AssertThat(ctx.Has<NonEmptyComponent>(id), Is().False());
			AssertThat(ctx.TryGet<NonEmptyComponent>(id), Equals(nullptr));
			AssertThat(NonEmptyComponent::destructed, Equals(1));
		});

		it("Can add many components", [&]() {
			EntityContext ctx;
			TArray<Id> ids{3};
			ctx.Create(ids);
			ctx.AddN(ids, NonEmptyComponent{2});

			for (Id id : ids)
			{
				auto* data = ctx.TryGet<NonEmptyComponent>(id);
				AssertThat(data, !Equals(nullptr));
				AssertThat(data->a, Equals(2));
			}
		});

		it("Can remove many components", [&]() {
			EntityContext ctx;
			TArray<Id> ids{3};
			ctx.Create(ids);
			ctx.AddN(ids, NonEmptyComponent{2});

			NonEmptyComponent::destructed = 0;
			TSpan<Id> firstTwo{ids.Data(), ids.Data() + 2};
			ctx.Remove<NonEmptyComponent>(firstTwo);
			AssertThat(NonEmptyComponent::destructed, Equals(2));
			AssertThat(ctx.TryGet<NonEmptyComponent>(ids[0]), Equals(nullptr));
			AssertThat(ctx.TryGet<NonEmptyComponent>(ids[1]), Equals(nullptr));
			AssertThat(ctx.TryGet<NonEmptyComponent>(ids[2]), !Equals(nullptr));

			// Repeat in different order
			ctx.AddN(ids, NonEmptyComponent{2});

			NonEmptyComponent::destructed = 0;
			TSpan<Id> lastTwo{ids.Data() + 1, ids.Data() + 3};
			ctx.Remove<NonEmptyComponent>(lastTwo);
			AssertThat(NonEmptyComponent::destructed, Equals(2));
			AssertThat(ctx.TryGet<NonEmptyComponent>(ids[0]), !Equals(nullptr));
			AssertThat(ctx.TryGet<NonEmptyComponent>(ids[1]), Equals(nullptr));
			AssertThat(ctx.TryGet<NonEmptyComponent>(ids[2]), Equals(nullptr));
		});

		it("Components are removed after node is deleted", [&]() {
			EntityContext ctx;
			Id id = ctx.Create();
			ctx.Add<EmptyComponent, NonEmptyComponent>(id);

			ctx.Destroy(id);
			AssertThat(ctx.IsValid(id), Is().False());

			AssertThat(ctx.Has<EmptyComponent>(id), Is().False());
			AssertThat(ctx.TryGet<EmptyComponent>(id), Equals(nullptr));
			AssertThat(ctx.Has<NonEmptyComponent>(id), Is().False());
			AssertThat(ctx.TryGet<NonEmptyComponent>(id), Equals(nullptr));
		});

		it("Components keep state when added", [&]() {
			EntityContext ctx;
			Id id = ctx.Create();
			ctx.AddN(id, NonEmptyComponent{2});
			AssertThat(ctx.TryGet<NonEmptyComponent>(id), !Equals(nullptr));
			AssertThat(ctx.Get<NonEmptyComponent>(id).a, Equals(2));
		});

		it("Can copy registry", []() {
			EntityContext ctxa;

			Id id = ctxa.Create();
			ctxa.Add<EmptyComponent, NonEmptyComponent>(id);
			Id id2 = ctxa.Create();
			ctxa.AddN(id2, NonEmptyComponent{2});

			EntityContext ctxb{ctxa};
			AssertThat(ctxb.Has<EmptyComponent>(id), Is().True());
			AssertThat(ctxb.Has<NonEmptyComponent>(id), Is().True());
			AssertThat(ctxb.TryGet<NonEmptyComponent>(id), !Equals(nullptr));

			// Holds component values
			AssertThat(ctxb.Get<NonEmptyComponent>(id2).a, Equals(2));
		});

		it("Can check components", [&]() {
			EntityContext ctx;
			Id id = NoId;
			AssertThat(ctx.Has<EmptyComponent>(id), Is().False());
			AssertThat(ctx.Has<NonEmptyComponent>(id), Is().False());

			id = ctx.Create();
			AssertThat(ctx.Has<EmptyComponent>(id), Is().False());
			AssertThat(ctx.Has<NonEmptyComponent>(id), Is().False());

			ctx.Add<EmptyComponent, NonEmptyComponent>(id);
			AssertThat(ctx.Has<EmptyComponent>(id), Is().True());
			AssertThat(ctx.Has<NonEmptyComponent>(id), Is().True());
		});

		it("Can destroy components on reset", [&]() {
			NonEmptyComponent::destructed = 0;
			TestComponent::destructed     = 0;

			EntityContext ctx;
			TArray<Id> ids{3};
			ctx.Create(ids);
			ctx.AddN(ids, NonEmptyComponent{2});
			ctx.AddN<TestComponent>(ids);

			ctx.Remove<NonEmptyComponent>(ids);
			ctx.Remove<TestComponent>(ids[0]);
			AssertThat(
			    NonEmptyComponent::destructed, Equals(4));       // 3 + 1 (passed by value on Add())
			AssertThat(TestComponent::destructed, Equals(2));    // 1 + 1 (passed by value on Add())

			NonEmptyComponent::destructed = 0;
			TestComponent::destructed     = 0;
			ctx.Reset();

			AssertThat(NonEmptyComponent::destructed, Equals(0));
			AssertThat(TestComponent::destructed, Equals(2));
		});

		it("Components are removed with the entity", [&]() {
			EntityContext ctx;
			Id id = ctx.Create();
			ctx.Add<EmptyComponent, NonEmptyComponent>(id);
			ctx.Destroy(id);

			AssertThat(ctx.Has<EmptyComponent>(id), Is().False());
			AssertThat(ctx.TryGet<EmptyComponent>(id), Equals(nullptr));
			AssertThat(ctx.Has<NonEmptyComponent>(id), Is().False());
			AssertThat(ctx.TryGet<NonEmptyComponent>(id), Equals(nullptr));
		});

		it("Can access components on recicled entities", [&]() {
			EntityContext ctx;
			Id id = ctx.Create();
			ctx.Add<EmptyComponent, NonEmptyComponent>(id);
			ctx.Destroy(id);

			id = ctx.Create();
			ctx.Add<NonEmptyComponent>(id);
			AssertThat(ctx.Has<EmptyComponent>(id), Is().False());
			AssertThat(ctx.Has<NonEmptyComponent>(id), Is().True());
			AssertThat(ctx.TryGet<NonEmptyComponent>(id), !Equals(nullptr));
		});
	});
});
