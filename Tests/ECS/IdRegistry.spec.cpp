// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <bandit/bandit.h>
#include <PipeECS.h>


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


go_bandit([]() {
	describe("ECS.IdRegistry", []() {
		it("Can create one id", [&]() {
			IdRegistry ids;
			AssertThat(ids.Size(), Equals(0));
			Id id = ids.Create();
			AssertThat(id, !Equals(NoId));
			AssertThat(ids.IsValid(id), Is().True());
			AssertThat(ids.Size(), Equals(1));
		});

		it("Can remove one id", [&]() {
			IdRegistry ids;
			Id id = ids.Create();
			AssertThat(ids.Size(), Equals(1));
			AssertThat(ids.Remove(id), Is().True());
			AssertThat(ids.IsValid(id), Is().False());
			AssertThat(ids.Size(), Equals(0));
		});

		it("Can create two and remove first", [&]() {
			IdRegistry ids;
			Id id1 = ids.Create();
			ids.Create();
			AssertThat(ids.Remove(id1), Is().True());
			AssertThat(ids.IsValid(id1), Is().False());
			AssertThat(ids.Size(), Equals(1));
		});

		it("Can create two and remove last", [&]() {
			IdRegistry ids;
			ids.Create();
			Id id2 = ids.Create();
			AssertThat(ids.Remove(id2), Is().True());
			AssertThat(ids.IsValid(id2), Is().False());
			AssertThat(ids.Size(), Equals(1));
		});

		it("Can remove one id (deferred)", [&]() {
			IdRegistry ids;
			Id id = ids.Create();
			AssertThat(ids.Size(), Equals(1));
			AssertThat(ids.DeferredRemove(id), Is().True());
			AssertThat(ids.IsValid(id), Is().False());
			AssertThat(ids.Size(), Equals(0));
		});

		it("Can create two and remove first (deferred)", [&]() {
			IdRegistry ids;
			Id id1 = ids.Create();
			ids.Create();
			AssertThat(ids.DeferredRemove(id1), Is().True());
			AssertThat(ids.IsValid(id1), Is().False());
			AssertThat(ids.Size(), Equals(1));
		});

		it("Can create two and remove last (deferred)", [&]() {
			IdRegistry ids;
			ids.Create();
			Id id2 = ids.Create();
			AssertThat(ids.DeferredRemove(id2), Is().True());
			AssertThat(ids.IsValid(id2), Is().False());
			AssertThat(ids.Size(), Equals(1));
		});

		it("Removed id index gets reused", [&]() {
			IdRegistry ids;
			ids.Create(1);
			Id id = ids.Create();
			ids.Create(1);
			AssertThat(ids.Remove(id), Is().True());
			Id id2 = ids.Create();
			AssertThat(id2.GetIndex(), Equals(id.GetIndex()));
			Id id3 = ids.Create();
			AssertThat(id3.GetIndex(), !Equals(id.GetIndex()));
		});

		it("Deferred removed id index doesn't get reused until flushed", [&]() {
			IdRegistry ids;
			ids.Create(1);
			Id id = ids.Create();
			ids.Create(1);
			AssertThat(ids.DeferredRemove(id), Is().True());
			Id id2 = ids.Create();
			AssertThat(id2.GetIndex(), !Equals(id.GetIndex()));
			ids.FlushDeferredRemoves();
			Id id3 = ids.Create();
			AssertThat(id3.GetIndex(), Equals(id.GetIndex()));
			Id id4 = ids.Create();
			AssertThat(id4.GetIndex(), !Equals(id.GetIndex()));
		});

		it("Can create many ids", [&]() {
			IdRegistry ids;
			AssertThat(ids.Size(), Equals(0));

			TArray<Id> list(3);
			ids.Create(list);

			AssertThat(ids.Size(), Equals(3));
			for (i32 i = 0; i < list.Size(); ++i)
			{
				AssertThat(list[i].GetIndex(), Equals(i));
				AssertThat(ids.IsValid(list[i]), Is().True());
			}
		});

		it("Can remove many ids", [&]() {
			IdRegistry ids;
			TArray<Id> list(3);
			ids.Create(list);
			AssertThat(ids.Size(), Equals(3));

			AssertThat(ids.Remove(list), Is().True());
			AssertThat(ids.Size(), Equals(0));

			for (i32 i = 0; i < list.Size(); ++i)
			{
				AssertThat(ids.IsValid(list[i]), Is().False());
			}
		});

		it("Can remove many ids (deferred)", [&]() {
			IdRegistry ids;
			TArray<Id> list(3);
			ids.Create(list);
			AssertThat(ids.Size(), Equals(3));

			AssertThat(ids.DeferredRemove(list), Is().True());
			AssertThat(ids.Size(), Equals(0));

			for (i32 i = 0; i < list.Size(); ++i)
			{
				AssertThat(ids.IsValid(list[i]), Is().False());
			}
		});
	});
});
