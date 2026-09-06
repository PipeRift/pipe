// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <Pipe/Memory/OwnPtr.h>
#include <PipeReflect.h>
#include <PipeTest.h>


using namespace p;

struct CastableBase : p::Castable
{
	p::TypeId ProvideTypeId() const override
	{
		return p::GetTypeId<CastableBase>();
	}
};

struct CastableDerived : CastableBase
{
	using Super = CastableBase;

	p::TypeId ProvideTypeId() const override
	{
		return p::GetTypeId<CastableDerived>();
	}
};

struct CastableLeaf : CastableDerived
{
	using Super = CastableDerived;

	p::TypeId ProvideTypeId() const override
	{
		return p::GetTypeId<CastableLeaf>();
	}
};

struct CastableCounter : p::Castable
{
	mutable i32 provideCalls = 0;

	p::TypeId ProvideTypeId() const override
	{
		++provideCalls;
		return p::GetTypeId<CastableCounter>();
	}
};


P_SPEC("Reflection.Castable", []()
{
	static_assert(p::IsCastable<CastableBase>);
	static_assert(p::IsCastable<CastableLeaf>);
	static_assert(!p::IsCastable<i32>);

	Describe("Basics", []()
	{
		It("Provides the compile-time type id", []()
		{
			CastableBase base;
			CastableDerived derived;

			Expect(base.GetTypeId()).ToEqual(p::GetTypeId<CastableBase>());
			Expect(derived.GetTypeId()).ToEqual(p::GetTypeId<CastableDerived>());
			Expect(derived.GetTypeId()).ToNotEqual(base.GetTypeId());
		});

		It("Computes and caches the type id lazily", []()
		{
			const CastableCounter counter;

			Expect(counter.provideCalls).ToEqual(0);

			const TypeId first = counter.GetTypeId();
			Expect(counter.provideCalls).ToEqual(1);

			const TypeId second = counter.GetTypeId();
			Expect(counter.provideCalls).ToEqual(1);
			Expect(second).ToEqual(first);
		});
	});

	Describe("Hierarchy", []()
	{
		BeforeEach([]()
		{
			// Registers the full chain: CastableLeaf -> CastableDerived -> CastableBase.
			p::RegisterTypeId<CastableLeaf>();
		});

		It("IsTypeParentOf reflects the registered hierarchy", []()
		{
			const TypeId baseId = p::GetTypeId<CastableBase>();
			const TypeId midId  = p::GetTypeId<CastableDerived>();
			const TypeId leafId = p::GetTypeId<CastableLeaf>();

			Expect(p::IsTypeParentOf(baseId, leafId)).ToEqual(true);
			Expect(p::IsTypeParentOf(midId, leafId)).ToEqual(true);
			Expect(p::IsTypeParentOf(baseId, midId)).ToEqual(true);

			Expect(p::IsTypeParentOf(leafId, baseId)).ToEqual(false);
			Expect(p::IsTypeParentOf(midId, baseId)).ToEqual(false);
		});

		It("Up-casts without checks", []()
		{
			auto leaf = p::MakeOwned<CastableLeaf>();

			Expect(p::Cast<CastableBase>(leaf.Get())).ToEqual(leaf.Get());
		});

		It("Down-casts only when the runtime type matches", []()
		{
			auto derived = p::MakeOwned<CastableDerived>();
			auto plain   = p::MakeOwned<CastableBase>();

			CastableBase* asBase = derived.Get();
			Expect(p::Cast<CastableDerived>(asBase)).ToEqual(derived.Get());

			Expect(p::Cast<CastableDerived>(plain.Get())).ToEqual(nullptr);
		});

		It("Down-casts through registered ancestors", []()
		{
			auto leaf = p::MakeOwned<CastableLeaf>();
			auto mid  = p::MakeOwned<CastableDerived>();

			// A base pointer to a leaf resolves to the intermediate type
			// through the registered parent chain.
			CastableBase* asBase = leaf.Get();
			Expect(p::Cast<CastableDerived>(asBase)).ToEqual(leaf.Get());

			// A derived instance is not a leaf.
			Expect(p::Cast<CastableLeaf>(mid.Get())).ToEqual(nullptr);
		});

		It("TTypeId binds compatible types", []()
		{
			const TypeId baseId = p::GetTypeId<CastableBase>();
			const TypeId leafId = p::GetTypeId<CastableLeaf>();

			// A derived runtime id can be bound to a base TTypeId. It keeps the
			// derived id, which matches the base via the parent chain.
			TTypeId<CastableBase> baseType{leafId};
			Expect(baseType.IsValid()).ToEqual(true);
			Expect(baseType).ToEqual(leafId);

			// A parent runtime id can't be bound to a child TTypeId.
			TTypeId<CastableLeaf> leafType{baseId};
			Expect(leafType.IsValid()).ToEqual(false);

			// Converting a child TTypeId to its base stays valid.
			TTypeId<CastableBase> converted{TTypeId<CastableDerived>{}};
			Expect(converted.IsValid()).ToEqual(true);
		});
	});
});