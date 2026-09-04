// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeECS.h>
#include <PipeTest.h>


using namespace p;
using namespace std::chrono_literals;


Spec("ECS.IdRegistry", []()
{
	It("Can create one id", []()
	{
		IdRegistry ids;
		Expect(ids.Size()).ToEqual(0);
		Id id = ids.Create();
		Expect(id).ToNotEqual(NoId);
		Expect(ids.IsValid(id)).ToBeTrue();
		Expect(ids.Size()).ToEqual(1);
	});

	It("Can remove one id", []()
	{
		IdRegistry ids;
		Id id = ids.Create();
		Expect(ids.Size()).ToEqual(1);
		Expect(ids.RemoveInstant(id)).ToBeTrue();
		Expect(ids.IsValid(id)).ToBeFalse();
		Expect(ids.Size()).ToEqual(0);
	});

	It("Can create two and remove first", []()
	{
		IdRegistry ids;
		Id id1 = ids.Create();
		ids.Create();
		Expect(ids.RemoveInstant(id1)).ToBeTrue();
		Expect(ids.IsValid(id1)).ToBeFalse();
		Expect(ids.Size()).ToEqual(1);
	});

	It("Can create two and remove last", []()
	{
		IdRegistry ids;
		ids.Create();
		Id id2 = ids.Create();
		Expect(ids.RemoveInstant(id2)).ToBeTrue();
		Expect(ids.IsValid(id2)).ToBeFalse();
		Expect(ids.Size()).ToEqual(1);
	});

	It("Can remove one id (deferred)", []()
	{
		IdRegistry ids;
		Id id = ids.Create();
		Expect(ids.Size()).ToEqual(1);
		Expect(ids.Remove(id)).ToBeTrue();
		Expect(ids.IsValid(id)).ToBeFalse();
		Expect(ids.Size()).ToEqual(0);
	});

	It("Can create two and remove first (deferred)", []()
	{
		IdRegistry ids;
		Id id1 = ids.Create();
		ids.Create();
		Expect(ids.Remove(id1)).ToBeTrue();
		Expect(ids.IsValid(id1)).ToBeFalse();
		Expect(ids.Size()).ToEqual(1);
	});

	It("Can create two and remove last (deferred)", []()
	{
		IdRegistry ids;
		ids.Create();
		Id id2 = ids.Create();
		Expect(ids.Remove(id2)).ToBeTrue();
		Expect(ids.IsValid(id2)).ToBeFalse();
		Expect(ids.Size()).ToEqual(1);
	});

	It("Removed id index gets reused", []()
	{
		IdRegistry ids;
		ids.Create();
		Id id = ids.Create();
		ids.Create();
		Expect(ids.RemoveInstant(id)).ToBeTrue();
		Id id2 = ids.Create();
		Expect(id2.GetIndex()).ToEqual(id.GetIndex());
		Id id3 = ids.Create();
		Expect(id3.GetIndex()).ToNotEqual(id.GetIndex());
	});

	It("Deferred removed id index doesn't get reused until flushed", []()
	{
		IdRegistry ids;
		ids.Create();
		Id id = ids.Create();
		ids.Create();
		Expect(ids.Remove(id)).ToBeTrue();
		Id id2 = ids.Create();
		Expect(id2.GetIndex()).ToNotEqual(id.GetIndex());
		ids.FlushDeferredRemovals();
		Id id3 = ids.Create();
		Expect(id3.GetIndex()).ToEqual(id.GetIndex());
		Id id4 = ids.Create();
		Expect(id4.GetIndex()).ToNotEqual(id.GetIndex());
	});

	It("Can create many ids", []()
	{
		IdRegistry ids;
		Expect(ids.Size()).ToEqual(0);

		TArray<Id> list(3);
		ids.Create(list);

		Expect(ids.Size()).ToEqual(3);
		for (i32 i = 0; i < list.Size(); ++i)
		{
			Expect(list[i].GetIndex()).ToEqual(i);
			Expect(ids.IsValid(list[i])).ToBeTrue();
		}
	});

	It("Can remove many ids", []()
	{
		IdRegistry ids;
		TArray<Id> list(3);
		ids.Create(list);
		Expect(ids.Size()).ToEqual(3);

		Expect(ids.RemoveInstant(list)).ToBeTrue();
		Expect(ids.Size()).ToEqual(0);

		for (i32 i = 0; i < list.Size(); ++i)
		{
			Expect(ids.IsValid(list[i])).ToBeFalse();
		}
	});

	It("Can remove many ids (deferred)", []()
	{
		IdRegistry ids;
		TArray<Id> list(3);
		ids.Create(list);
		Expect(ids.Size()).ToEqual(3);

		Expect(ids.Remove(list)).ToBeTrue();
		Expect(ids.Size()).ToEqual(0);

		for (i32 i = 0; i < list.Size(); ++i)
		{
			Expect(ids.IsValid(list[i])).ToBeFalse();
		}
	});
});
