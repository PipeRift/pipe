// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <Pipe/Memory/OwnPtr.h>


using namespace p;


template<typename T>
struct TestPtrBuilder : p::TPtrBuilder<T>
{
	template<typename... Args>
	static T* New(Arena& arena, Args&&... args)
	{
		T* ptr          = new (p::Alloc<T>(arena)) T(Fwd<Args>(args)...);
		ptr->bCalledNew = true;
		return ptr;
	}

	static void Delete(Arena& arena, void* rawPtr)
	{
		T::bCalledDelete = true;
		T* const ptr     = static_cast<T*>(rawPtr);
		ptr->~T();
		p::Free<T>(arena, ptr);
	}
};


struct EmptyStruct
{};

struct MockStruct
{
	template<typename T>
	using PtrBuilder = TestPtrBuilder<T>;

	bool bCalledNew = false;
	inline static bool bCalledDelete = false;
};


namespace
{
// Auto-registers via static init (macro-free go_bandit equivalent).
const bool autoRegistered = []()
{
Spec("Core.OwnPtr", []()
{
	Describe("Owner pointer", []()
	{
		It("Can initialize to empty", []()
		{
			TOwnPtr<EmptyStruct> ptr;
			Expect(ptr.IsValid()).ToEqual(false);
			Expect(ptr.Get()).ToEqual(nullptr);
		});

		It("Can instantiate", []()
		{
			TOwnPtr<EmptyStruct> ptr = MakeOwned<EmptyStruct>();
			Expect(ptr.IsValid()).ToEqual(true);
			Expect(ptr.Get()).ToNotEqual(nullptr);
		});

		It("Owner can release", []()
		{
			TOwnPtr<EmptyStruct> owner = MakeOwned<EmptyStruct>();
			Expect(owner.IsValid()).ToEqual(true);

			owner.Delete();
			Expect(owner.IsValid()).ToEqual(false);
		});

		It("Owner is released when destroyed", []()
		{
			TPtr<EmptyStruct> ptr;
			{
				TOwnPtr<EmptyStruct> owner = MakeOwned<EmptyStruct>();

				ptr = owner;
				Expect(ptr.IsValid()).ToEqual(true);
			}
			Expect(ptr.IsValid()).ToEqual(false);
		});

		Describe("Ptr Builder", []()
		{
			It("Calls custom new", []()
			{
				auto owner = MakeOwned<MockStruct>();
				Expect(owner->bCalledNew).ToEqual(true);
			});

			It("Calls custom delete", []()
			{
				MockStruct::bCalledDelete = false;
				auto owner                = MakeOwned<MockStruct>();
				Expect(MockStruct::bCalledDelete).ToEqual(false);
				owner.Delete();
				Expect(MockStruct::bCalledDelete).ToEqual(true);
			});
		});
	});

	Describe("Weak pointer", []()
	{
		It("Can initialize to empty", []()
		{
			TPtr<EmptyStruct> ptr;
			Expect(ptr.IsValid()).ToEqual(false);
			Expect(ptr.Get()).ToEqual(nullptr);
		});

		It("Can initialize from owner", []()
		{
			TOwnPtr<EmptyStruct> owner = MakeOwned<EmptyStruct>();
			TPtr<EmptyStruct> ptr      = owner;

			Expect(ptr.IsValid()).ToEqual(true);
			Expect(ptr.Get()).ToNotEqual(nullptr);
		});

		It("Can copy from other weak", []()
		{
			TOwnPtr<EmptyStruct> owner = MakeOwned<EmptyStruct>();
			auto* raw                  = owner.Get();
			TPtr<EmptyStruct> ptr      = owner;
			TPtr<EmptyStruct> ptr2     = ptr;

			Expect(ptr2.IsValid()).ToEqual(true);
			Expect(ptr.Get()).ToEqual(raw);
			Expect(ptr2.Get()).ToEqual(raw);
		});

		It("Can move from other weak", []()
		{
			TOwnPtr<EmptyStruct> owner = MakeOwned<EmptyStruct>();
			auto* raw                  = owner.Get();
			auto weak                  = owner.AsPtr();
			auto movedWeak             = Move(weak);

			Expect(weak.IsValid()).ToEqual(false);
			Expect(movedWeak.IsValid()).ToEqual(true);

			Expect(weak.Get()).ToEqual(nullptr);
			Expect(movedWeak.Get()).ToEqual(raw);
		});

		It("Ptr is null after IsValid() == false", []()
		{
			TOwnPtr<EmptyStruct> owner = MakeOwned<EmptyStruct>();
			TPtr<EmptyStruct> ptr      = owner;
			owner.Delete();

			Expect(ptr.Get()).ToNotEqual(nullptr);

			Expect(ptr.IsValid()).ToEqual(false);
			Expect(ptr.Get()).ToEqual(nullptr);
		});
	});

	Describe("Comparisons", []()
	{
		It("Owner can equal Owner", []()
		{
			auto owner  = MakeOwned<EmptyStruct>();
			auto owner2 = MakeOwned<EmptyStruct>();
			TOwnPtr<EmptyStruct> ownerEmpty;

			Expect(owner == owner).ToEqual(true);
			Expect(owner == owner2).ToEqual(false);
			Expect(ownerEmpty == ownerEmpty).ToEqual(true);
			Expect(owner == ownerEmpty).ToEqual(false);

			Expect(owner != owner).ToEqual(false);
			Expect(owner != owner2).ToEqual(true);
			Expect(ownerEmpty != ownerEmpty).ToEqual(false);
			Expect(owner != ownerEmpty).ToEqual(true);
		});

		It("Owner can equal Weak", []()
		{
			auto owner  = MakeOwned<EmptyStruct>();
			auto owner2 = MakeOwned<EmptyStruct>();
			auto weak   = owner.AsPtr();
			TOwnPtr<EmptyStruct> ownerEmpty;
			TPtr<EmptyStruct> weakEmpty;

			Expect(owner == weak).ToEqual(true);
			Expect(owner2 == weak).ToEqual(false);
			Expect(ownerEmpty == weak).ToEqual(false);
			Expect(ownerEmpty == weakEmpty).ToEqual(true);

			Expect(owner != weak).ToEqual(false);
			Expect(owner2 != weak).ToEqual(true);
			Expect(ownerEmpty != weak).ToEqual(true);
			Expect(ownerEmpty != weakEmpty).ToEqual(false);
		});

		It("Weak can equal Weak", []()
		{
			auto owner  = MakeOwned<EmptyStruct>();
			auto owner2 = MakeOwned<EmptyStruct>();
			auto weak   = owner.AsPtr();
			auto weak2  = owner2.AsPtr();
			TPtr<EmptyStruct> weakEmpty;

			Expect(weak == weak).ToEqual(true);
			Expect(weak2 == weak).ToEqual(false);
			Expect(weakEmpty == weak).ToEqual(false);
			Expect(weakEmpty == weakEmpty).ToEqual(true);

			Expect(weak != weak).ToEqual(false);
			Expect(weak2 != weak).ToEqual(true);
			Expect(weakEmpty != weak).ToEqual(true);
			Expect(weakEmpty != weakEmpty).ToEqual(false);
		});

		It("Weak can equal Owner", []()
		{
			auto owner  = MakeOwned<EmptyStruct>();
			auto owner2 = MakeOwned<EmptyStruct>();
			auto weak   = owner.AsPtr();
			auto weak2  = owner2.AsPtr();
			TOwnPtr<EmptyStruct> ownerEmpty;
			TPtr<EmptyStruct> weakEmpty;

			Expect(weak == owner).ToEqual(true);
			Expect(weak2 == owner).ToEqual(false);
			Expect(weakEmpty == owner).ToEqual(false);
			Expect(weakEmpty == ownerEmpty).ToEqual(true);

			Expect(weak != owner).ToEqual(false);
			Expect(weak2 != owner).ToEqual(true);
			Expect(weakEmpty != owner).ToEqual(true);
			Expect(weakEmpty != ownerEmpty).ToEqual(false);
		});
	});

	Describe("Counter", []()
	{
		It("Adds weaks", []()
		{
			auto owner          = MakeOwned<EmptyStruct>();
			const auto* counter = owner.GetCounter();
			Expect(counter->weakCount).ToEqual(0u);

			auto weak = owner.AsPtr();
			Expect(counter->weakCount).ToEqual(1u);
		});

		It("Removes weaks", []()
		{
			auto owner          = MakeOwned<EmptyStruct>();
			const auto* counter = owner.GetCounter();
			{
				auto weak = owner.AsPtr();
				Expect(counter->weakCount).ToEqual(1u);
			}
			Expect(counter->weakCount).ToEqual(0u);
		});

		It("Removes with owner release", []()
		{
			auto owner = MakeOwned<EmptyStruct>();
			Expect(owner.GetCounter()).ToNotEqual(nullptr);

			owner.Delete();
			Expect(owner.GetCounter()).ToEqual(nullptr);
		});

		It("Removes with no weakCount left", []()
		{
			auto owner = MakeOwned<EmptyStruct>();
			auto weak  = owner.AsPtr();
			Expect(weak.GetCounter()).ToNotEqual(nullptr);

			owner.Delete();
			Expect(weak.GetCounter()).ToNotEqual(nullptr);

			weak.Reset();
			Expect(owner.GetCounter()).ToEqual(nullptr);
		});
	});


	It("Can detect custom PtrBuilders", []()
	{
		Expect(p::HasCustomPtrBuilder<EmptyStruct>::value).ToEqual(false);
		Expect(p::HasCustomPtrBuilder<MockStruct>::value).ToEqual(true);
	});

	Describe("Typeless pointer", []()
	{
		It("Can convert to OwnPtr from TOwnPtr", []()
		{
			TOwnPtr<EmptyStruct> typedPtr = MakeOwned<EmptyStruct>();
			Expect(typedPtr.IsValid()).ToEqual(true);

			EmptyStruct* data = typedPtr.Get();

			OwnPtr ptr = Move(typedPtr);
			Expect(typedPtr.IsValid()).ToEqual(false);
			Expect(ptr.IsValid()).ToEqual(true);
			Expect(ptr.Get()).ToEqual(data);
			Expect(ptr.Get<EmptyStruct>()).ToEqual(data);
		});

		It("Can convert to TOwnPtr from OwnPtr", []()
		{
			OwnPtr ptr = MakeOwned<EmptyStruct>();
			Expect(ptr.IsValid()).ToEqual(true);
			auto* data = ptr.Get<EmptyStruct>();

			TOwnPtr<EmptyStruct> typedPtr = Move(ptr);
			Expect(ptr.IsValid()).ToEqual(false);
			Expect(typedPtr.IsValid()).ToEqual(true);
			Expect(typedPtr.Get()).ToEqual(data);
		});

		It("Can move", []()
		{
			OwnPtr ptr1 = MakeOwned<EmptyStruct>();
			Expect(ptr1.IsValid()).ToEqual(true);
			Expect(ptr1.GetId()).ToEqual(GetTypeId<EmptyStruct>());
			auto* data = ptr1.Get<EmptyStruct>();

			OwnPtr ptr2 = Move(ptr1);
			Expect(ptr1.IsValid()).ToEqual(false);
			Expect(ptr1.Get<EmptyStruct>()).ToEqual(nullptr);
			Expect(ptr1.GetId()).ToEqual(TypeId::None());

			Expect(ptr2.IsValid()).ToEqual(true);
			Expect(ptr2.Get<EmptyStruct>()).ToEqual(data);
			Expect(ptr2.GetId()).ToEqual(GetTypeId<EmptyStruct>());
		});

		It("Cant retrive invalid types", []()
		{
			OwnPtr ptr = MakeOwned<EmptyStruct>();
			Expect(ptr.Get<EmptyStruct>()).ToNotEqual(nullptr);
			Expect(ptr.Get<MockStruct>()).ToEqual(nullptr);
		});
	});
});
return true;
}();
}    // namespace
