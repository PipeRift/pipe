// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <PipeECS.h>


using namespace p;
using namespace std::chrono_literals;


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


Spec("ECS.Components", []()
{
It("Can add one component", []()
{
	IdContext ctx;
	Id id = AddId(ctx);
	Expect(ctx.Has<EmptyComponent>(id)).ToBeFalse();
	Expect(ctx.TryGet<EmptyComponent>(id)).ToEqual(nullptr);
	Expect(ctx.TryGet<NonEmptyComponent>(id)).ToEqual(nullptr);

	ctx.Add<EmptyComponent>(id);
	Expect(ctx.Has<EmptyComponent>(id)).ToBeTrue();
	Expect(ctx.TryGet<EmptyComponent>(id)).ToEqual(nullptr);

	ctx.Add<NonEmptyComponent>(id);
	Expect(ctx.Has<NonEmptyComponent>(id)).ToBeTrue();
	Expect(ctx.TryGet<NonEmptyComponent>(id)).ToNotEqual(nullptr);
});

It("Can remove one component", []()
{
	IdContext ctx;
	Id id = AddId(ctx);
	ctx.Add<EmptyComponent, NonEmptyComponent>(id);

	ctx.Remove<EmptyComponent>(id);
	Expect(ctx.Has<EmptyComponent>(id)).ToBeFalse();
	Expect(ctx.TryGet<EmptyComponent>(id)).ToEqual(nullptr);

	NonEmptyComponent::destructed = 0;
	ctx.Remove<NonEmptyComponent>(id);
	Expect(ctx.Has<NonEmptyComponent>(id)).ToBeFalse();
	Expect(ctx.TryGet<NonEmptyComponent>(id)).ToEqual(nullptr);
	Expect(NonEmptyComponent::destructed).ToEqual(1);
});

It("Can add many components", []()
{
	IdContext ctx;
	TArray<Id> ids{3};
	AddId(ctx, ids);
	ctx.AddN(ids, NonEmptyComponent{2});

	for (Id id : ids)
	{
		auto* data = ctx.TryGet<NonEmptyComponent>(id);
		Expect(data).ToNotEqual(nullptr);
		Expect(data->a).ToEqual(2);
	}
});

It("Can remove many components", []()
{
	IdContext ctx;
	TArray<Id> ids{3};
	AddId(ctx, ids);
	ctx.AddN(ids, NonEmptyComponent{2});

	NonEmptyComponent::destructed = 0;
	TView<Id> firstTwo{ids.Data(), ids.Data() + 2};
	ctx.Remove<NonEmptyComponent>(firstTwo);
	Expect(NonEmptyComponent::destructed).ToEqual(2);
	Expect(ctx.TryGet<NonEmptyComponent>(ids[0])).ToEqual(nullptr);
	Expect(ctx.TryGet<NonEmptyComponent>(ids[1])).ToEqual(nullptr);
	Expect(ctx.TryGet<NonEmptyComponent>(ids[2])).ToNotEqual(nullptr);

	// Repeat in different order
	ctx.AddN(ids, NonEmptyComponent{2});

	NonEmptyComponent::destructed = 0;
	TView<Id> lastTwo{ids.Data() + 1, ids.Data() + 3};
	ctx.Remove<NonEmptyComponent>(lastTwo);
	Expect(NonEmptyComponent::destructed).ToEqual(2);
	Expect(ctx.TryGet<NonEmptyComponent>(ids[0])).ToNotEqual(nullptr);
	Expect(ctx.TryGet<NonEmptyComponent>(ids[1])).ToEqual(nullptr);
	Expect(ctx.TryGet<NonEmptyComponent>(ids[2])).ToEqual(nullptr);
});

It("Components are removed after node is deleted", []()
{
	IdContext ctx;
	Id id = AddId(ctx);
	ctx.Add<EmptyComponent, NonEmptyComponent>(id);

	RmId(ctx, id, p::RmIdFlags::Instant);
	Expect(ctx.IsValid(id)).ToBeFalse();

	Expect(ctx.Has<EmptyComponent>(id)).ToBeFalse();
	Expect(ctx.TryGet<EmptyComponent>(id)).ToEqual(nullptr);
	Expect(ctx.Has<NonEmptyComponent>(id)).ToBeFalse();
	Expect(ctx.TryGet<NonEmptyComponent>(id)).ToEqual(nullptr);
});

It("Components are removed after node is deleted (deferred)", []()
{
	IdContext ctx;
	Id id = AddId(ctx);
	ctx.Add<EmptyComponent, NonEmptyComponent>(id);

	RmId(ctx, id);
	Expect(ctx.IsValid(id)).ToBeFalse();

	Expect(ctx.Has<EmptyComponent>(id)).ToBeTrue();
	Expect(ctx.TryGet<EmptyComponent>(id)).ToEqual(nullptr);
	Expect(ctx.Has<NonEmptyComponent>(id)).ToBeTrue();
	Expect(ctx.TryGet<NonEmptyComponent>(id)).ToNotEqual(nullptr);

	FlushDeferredRemovals(ctx);
	Expect(ctx.Has<EmptyComponent>(id)).ToBeFalse();
	Expect(ctx.TryGet<EmptyComponent>(id)).ToEqual(nullptr);
	Expect(ctx.Has<NonEmptyComponent>(id)).ToBeFalse();
	Expect(ctx.TryGet<NonEmptyComponent>(id)).ToEqual(nullptr);
});

It("Components keep state when added", []()
{
	IdContext ctx;
	Id id = AddId(ctx);
	ctx.AddN(id, NonEmptyComponent{2});
	Expect(ctx.TryGet<NonEmptyComponent>(id)).ToNotEqual(nullptr);
	Expect(ctx.Get<NonEmptyComponent>(id).a).ToEqual(2);
});

It("Can copy registry", []()
{
	IdContext ctxa;

	Id id = AddId(ctxa);
	ctxa.Add<EmptyComponent, NonEmptyComponent>(id);
	Id id2 = AddId(ctxa);
	ctxa.AddN(id2, NonEmptyComponent{2});

	IdContext ctxb{ctxa};
	Expect(ctxb.Has<EmptyComponent>(id)).ToBeTrue();
	Expect(ctxb.Has<NonEmptyComponent>(id)).ToBeTrue();
	Expect(ctxb.TryGet<NonEmptyComponent>(id)).ToNotEqual(nullptr);

	// Holds component values
	Expect(ctxb.Has<NonEmptyComponent>(id2)).ToBeTrue();
	Expect(ctxb.Get<NonEmptyComponent>(id2).a).ToEqual(2);
});

It("Can check components", []()
{
	IdContext ctx;
	Id id = NoId;
	Expect(ctx.Has<EmptyComponent>(id)).ToBeFalse();
	Expect(ctx.Has<NonEmptyComponent>(id)).ToBeFalse();

	id = AddId(ctx);
	Expect(ctx.Has<EmptyComponent>(id)).ToBeFalse();
	Expect(ctx.Has<NonEmptyComponent>(id)).ToBeFalse();

	ctx.Add<EmptyComponent, NonEmptyComponent>(id);
	Expect(ctx.Has<EmptyComponent>(id)).ToBeTrue();
	Expect(ctx.Has<NonEmptyComponent>(id)).ToBeTrue();
});

It("Can destroy components on reset", []()
{
	NonEmptyComponent::destructed = 0;
	TestComponent::destructed     = 0;

	IdContext ctx;
	TArray<Id> ids{3};
	AddId(ctx, ids);
	ctx.AddN(ids, NonEmptyComponent{2});
	ctx.AddN<TestComponent>(ids);

	ctx.Remove<NonEmptyComponent>(ids);
	ctx.Remove<TestComponent>(ids[0]);
	Expect(
	    NonEmptyComponent::destructed).ToEqual(4);       // 3 + 1 (passed by value on Add())
	Expect(TestComponent::destructed).ToEqual(2);    // 1 + 1 (passed by value on Add())

	NonEmptyComponent::destructed = 0;
	TestComponent::destructed     = 0;
	ctx.Reset();

	Expect(NonEmptyComponent::destructed).ToEqual(0);
	Expect(TestComponent::destructed).ToEqual(2);
});

It("Components are removed with the entity", []()
{
	IdContext ctx;
	Id id = AddId(ctx);
	ctx.Add<EmptyComponent, NonEmptyComponent>(id);
	RmId(ctx, id, p::RmIdFlags::Instant);
	Expect(ctx.IsValid(id)).ToBeFalse();

	Expect(ctx.Has<EmptyComponent>(id)).ToBeFalse();
	Expect(ctx.TryGet<EmptyComponent>(id)).ToEqual(nullptr);
	Expect(ctx.Has<NonEmptyComponent>(id)).ToBeFalse();
	Expect(ctx.TryGet<NonEmptyComponent>(id)).ToEqual(nullptr);
});

It("Components are removed with the entity (deferred)", []()
{
	IdContext ctx;
	Id id = AddId(ctx);
	ctx.Add<EmptyComponent, NonEmptyComponent>(id);
	RmId(ctx, id);
	Expect(ctx.IsValid(id)).ToBeFalse();

	Expect(ctx.Has<EmptyComponent>(id)).ToBeTrue();
	Expect(ctx.TryGet<EmptyComponent>(id)).ToEqual(nullptr);
	Expect(ctx.Has<NonEmptyComponent>(id)).ToBeTrue();
	Expect(ctx.TryGet<NonEmptyComponent>(id)).ToNotEqual(nullptr);

	FlushDeferredRemovals(ctx);
	Expect(ctx.Has<EmptyComponent>(id)).ToBeFalse();
	Expect(ctx.TryGet<EmptyComponent>(id)).ToEqual(nullptr);
	Expect(ctx.Has<NonEmptyComponent>(id)).ToBeFalse();
	Expect(ctx.TryGet<NonEmptyComponent>(id)).ToEqual(nullptr);
});

It("Can access components on recicled entities", []()
{
	IdContext ctx;
	Id id = AddId(ctx);
	ctx.Add<EmptyComponent, NonEmptyComponent>(id);
	RmId(ctx, id);

	id = AddId(ctx);
	ctx.Add<NonEmptyComponent>(id);
	Expect(ctx.Has<EmptyComponent>(id)).ToBeFalse();
	Expect(ctx.Has<NonEmptyComponent>(id)).ToBeTrue();
	Expect(ctx.TryGet<NonEmptyComponent>(id)).ToNotEqual(nullptr);
});

It("Can access CRemoved", []()
{
	IdContext ctx;
	Id id = AddId(ctx);
	ctx.Add<EmptyComponent, NonEmptyComponent>(id);
	RmId(ctx, id);

	Expect(ctx.Has<CRemoved>(id)).ToBeTrue();
	Expect(ctx.Has<EmptyComponent>(id)).ToBeTrue();
	Expect(ctx.Has<NonEmptyComponent>(id)).ToBeTrue();
	Expect(ctx.TryGet<NonEmptyComponent>(id)).ToNotEqual(nullptr);
});
});
