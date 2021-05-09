// Copyright 2015-2021 Piperift - All rights reserved

#include <Memory/OwnPtr.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;


template <typename T>
struct TestPtrBuilder : Rift::TPtrBuilder<T>
{
	template <typename... Args>
	static T* New(Args&&... args)
	{
		T* ptr = new T(std::forward<Args>(args)...);
		ptr->bCalledNew = true;
		return ptr;
	}

	static void Delete(void* ptr)
	{
		T::bCalledDelete = true;
		delete static_cast<T*>(ptr);
	}
};


struct EmptyStruct
{};

struct MockStruct
{
	template <typename T>
	using PtrBuilder = TestPtrBuilder<T>;

	bool bCalledNew = false;
	static bool bCalledDelete;
};


go_bandit([]() {
	describe("OwnPtr", []() {
		describe("Owner pointer", []() {
			it("Can initialize to empty", [&]() {
				TOwnPtr<EmptyStruct> ptr;
				AssertThat(ptr.IsValid(), Equals(false));
				AssertThat(ptr.Get(), Equals(nullptr));
			});

			it("Can instantiate", [&]() {
				TOwnPtr<EmptyStruct> ptr = MakeOwned<EmptyStruct>();
				AssertThat(ptr.IsValid(), Equals(true));
				AssertThat(ptr.Get(), Is().Not().EqualTo(nullptr));
			});

			it("Owner can release", [&]() {
				TOwnPtr<EmptyStruct> owner = MakeOwned<EmptyStruct>();
				AssertThat(owner.IsValid(), Equals(true));

				owner.Delete();
				AssertThat(owner.IsValid(), Equals(false));
			});

			it("Owner is released when destroyed", [&]() {
				TPtr<EmptyStruct> ptr;
				{
					TOwnPtr<EmptyStruct> owner = MakeOwned<EmptyStruct>();

					ptr = owner;
					AssertThat(ptr.IsValid(), Equals(true));
				}
				AssertThat(ptr.IsValid(), Equals(false));
			});

			describe("Ptr Builder", []() {
				it("Calls custom new", [&]() {
					auto owner = MakeOwned<MockStruct>();
					AssertThat(owner->bCalledNew, Equals(true));
				});

				it("Calls custom delete", [&]() {
					MockStruct::bCalledDelete = false;
					auto owner                = MakeOwned<MockStruct>();
					AssertThat(MockStruct::bCalledDelete, Equals(false));
					owner.Delete();
					AssertThat(MockStruct::bCalledDelete, Equals(true));
				});
			});
		});

		describe("Weak pointer", []() {
			it("Can initialize to empty", [&]() {
				TPtr<EmptyStruct> ptr;
				AssertThat(ptr.IsValid(), Equals(false));
				AssertThat(ptr.Get(), Equals(nullptr));
			});

			it("Can initialize from owner", [&]() {
				TOwnPtr<EmptyStruct> owner = MakeOwned<EmptyStruct>();
				TPtr<EmptyStruct> ptr      = owner;

				AssertThat(ptr.IsValid(), Equals(true));
				AssertThat(ptr.Get(), Is().Not().EqualTo(nullptr));
			});

			it("Can copy from other weak", [&]() {
				TOwnPtr<EmptyStruct> owner = MakeOwned<EmptyStruct>();
				auto* raw                  = owner.GetUnsafe();
				TPtr<EmptyStruct> ptr      = owner;
				TPtr<EmptyStruct> ptr2    = ptr;

				AssertThat(ptr2.IsValid(), Equals(true));
				AssertThat(ptr.Get(), Equals(raw));
				AssertThat(ptr2.Get(), Equals(raw));
			});

			it("Can move from other weak", [&]() {
				TOwnPtr<EmptyStruct> owner = MakeOwned<EmptyStruct>();
				auto* raw                  = owner.GetUnsafe();
				auto weak                  = owner.AsPtr();
				auto movedWeak             = Move(weak);

				AssertThat(weak.IsValid(), Equals(false));
				AssertThat(movedWeak.IsValid(), Equals(true));

				AssertThat(weak.Get(), Equals(nullptr));
				AssertThat(movedWeak.Get(), Equals(raw));
			});

			it("Ptr is null after IsValid() == false", [&]() {
				TOwnPtr<EmptyStruct> owner = MakeOwned<EmptyStruct>();
				TPtr<EmptyStruct> ptr      = owner;
				owner.Delete();

				AssertThat(ptr.GetUnsafe(), Is().Not().EqualTo(nullptr));

				AssertThat(ptr.IsValid(), Equals(false));
				AssertThat(ptr.GetUnsafe(), Equals(nullptr));
			});
		});

		describe("Comparisons", []() {
			it("Owner can equal Owner", [&]() {
				auto owner = MakeOwned<EmptyStruct>();
				auto owner2 = MakeOwned<EmptyStruct>();
				TOwnPtr<EmptyStruct> ownerEmpty;

				AssertThat(owner == owner, Equals(true));
				AssertThat(owner == owner2, Equals(false));
				AssertThat(ownerEmpty == ownerEmpty, Equals(true));
				AssertThat(owner == ownerEmpty, Equals(false));

				AssertThat(owner != owner, Equals(false));
				AssertThat(owner != owner2, Equals(true));
				AssertThat(ownerEmpty != ownerEmpty, Equals(false));
				AssertThat(owner != ownerEmpty, Equals(true));
			});

			it("Owner can equal Weak", [&]() {
				auto owner = MakeOwned<EmptyStruct>();
				auto owner2 = MakeOwned<EmptyStruct>();
				auto weak   = owner.AsPtr();
				TOwnPtr<EmptyStruct> ownerEmpty;
				TPtr<EmptyStruct> weakEmpty;

				AssertThat(owner == weak, Equals(true));
				AssertThat(owner2 == weak, Equals(false));
				AssertThat(ownerEmpty == weak, Equals(false));
				AssertThat(ownerEmpty == weakEmpty, Equals(true));

				AssertThat(owner != weak, Equals(false));
				AssertThat(owner2 != weak, Equals(true));
				AssertThat(ownerEmpty != weak, Equals(true));
				AssertThat(ownerEmpty != weakEmpty, Equals(false));
			});

			it("Weak can equal Weak", [&]() {
				auto owner = MakeOwned<EmptyStruct>();
				auto owner2 = MakeOwned<EmptyStruct>();
				auto weak   = owner.AsPtr();
				auto weak2  = owner2.AsPtr();
				TPtr<EmptyStruct> weakEmpty;

				AssertThat(weak == weak, Equals(true));
				AssertThat(weak2 == weak, Equals(false));
				AssertThat(weakEmpty == weak, Equals(false));
				AssertThat(weakEmpty == weakEmpty, Equals(true));

				AssertThat(weak != weak, Equals(false));
				AssertThat(weak2 != weak, Equals(true));
				AssertThat(weakEmpty != weak, Equals(true));
				AssertThat(weakEmpty != weakEmpty, Equals(false));
			});

			it("Weak can equal Owner", [&]() {
				auto owner = MakeOwned<EmptyStruct>();
				auto owner2 = MakeOwned<EmptyStruct>();
				auto weak   = owner.AsPtr();
				auto weak2  = owner2.AsPtr();
				TOwnPtr<EmptyStruct> ownerEmpty;
				TPtr<EmptyStruct> weakEmpty;

				AssertThat(weak == owner, Equals(true));
				AssertThat(weak2 == owner, Equals(false));
				AssertThat(weakEmpty == owner, Equals(false));
				AssertThat(weakEmpty == ownerEmpty, Equals(true));

				AssertThat(weak != owner, Equals(false));
				AssertThat(weak2 != owner, Equals(true));
				AssertThat(weakEmpty != owner, Equals(true));
				AssertThat(weakEmpty != ownerEmpty, Equals(false));
			});
		});

		describe("Counter", []() {
			it("Adds weaks", [&]() {
				auto owner = MakeOwned<EmptyStruct>();
				const auto* counter = owner.GetCounter();
				AssertThat(counter->weakCount, Equals(0u));

				auto weak = owner.AsPtr();
				AssertThat(counter->weakCount, Equals(1u));
			});

			it("Removes weaks", [&]() {
				auto owner = MakeOwned<EmptyStruct>();
				const auto* counter = owner.GetCounter();
				{
					auto weak = owner.AsPtr();
					AssertThat(counter->weakCount, Equals(1u));
				}
				AssertThat(counter->weakCount, Equals(0u));
			});

			it("Removes with owner release", [&]() {
				auto owner = MakeOwned<EmptyStruct>();
				AssertThat(owner.GetCounter(), Is().Not().EqualTo(nullptr));

				owner.Delete();
				AssertThat(owner.GetCounter(), Equals(nullptr));
			});

			it("Removes with no weakCount left", [&]() {
				auto owner = MakeOwned<EmptyStruct>();
				auto weak  = owner.AsPtr();
				AssertThat(weak.GetCounter(), Is().Not().EqualTo(nullptr));

				owner.Delete();
				AssertThat(weak.GetCounter(), Is().Not().EqualTo(nullptr));

				weak.Reset();
				AssertThat(owner.GetCounter(), Equals(nullptr));
			});
		});


		it("Can detect custom PtrBuilders", [&]() {
			AssertThat(Rift::HasCustomPtrBuilder<EmptyStruct>::value, Equals(false));
			AssertThat(Rift::HasCustomPtrBuilder<MockStruct>::value, Equals(true));
		});
	});
});

inline bool MockStruct::bCalledDelete = false;
