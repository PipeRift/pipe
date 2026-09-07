// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Memory/OwnPtr.h"
#include "PipeReflect.h"
#include "PipeType.h"

#include <utility>


namespace p
{
	class P_API BaseObject : public Castable
	{
	protected:
		BaseObject() = default;

	public:
		virtual ~BaseObject() = default;

		TPtr<Object> AsPtr() const;
	};


	// For shared export purposes, we separate pointers from the exported Class
	struct P_API ObjectOwnership
	{
		TPtr<BaseObject> self;
		TPtr<BaseObject> owner;
		static TPtr<BaseObject> nextOwner;


		ObjectOwnership();
		const TPtr<BaseObject>& AsPtr() const;
		const TPtr<BaseObject>& GetOwner() const;
	};


	template<typename T>
	struct TObjectPtrBuilder : public TPtrBuilder<T>
	{
		template<typename... Args>
		static T* New(Arena& arena, Args&&... args, const TPtr<BaseObject>& owner = {})
		{
			// Sets owner during construction
			// TODO: Fix self not existing at the moment of construction
			ObjectOwnership::nextOwner = owner;
			return new (p::Alloc<T>(arena)) T(std::forward<Args>(args)...);
		}

		// Allow creation of classes using reflection
		static T* New(Arena& arena, TypeId type, TPtr<BaseObject> owner = {})
		{
			if (GetTypeId<T>() == type || IsTypeParentOf(GetTypeId<T>(), type))
			{
				if (auto* ops = GetTypeObjectOps(type))
				{
					// Sets owner during construction
					// TODO: Fix self not existing at the moment of construction
					ObjectOwnership::nextOwner = owner;
					return Cast<T>(ops->onNew(arena));
				}
			}
			return nullptr;
		}

		static void Delete(Arena& arena, void* rawPtr)
		{
			T* ptr               = static_cast<T*>(rawPtr);
			const sizet typeSize = GetTypeSize(ptr->GetTypeId());
			ptr->~T();
			arena.Free((void*)ptr, typeSize);    // size depends on inheritance!
		}
	};


	class P_API Object : public BaseObject
	{
	public:
		using Self = Object;
		template<typename T>
		using PtrBuilder = TObjectPtrBuilder<T>;

		p::TypeId ProvideTypeId() const override
		{
			return p::GetTypeId<Object>();
		}

		static constexpr p::TypeFlags staticFlags = TF_None;

		P_REFLECTION_BODY({})

	private:
		ObjectOwnership ownership;


	public:
		Object() = default;

		void ChangeOwner(const TPtr<BaseObject>& inOwner);
		template<typename T = Object>
		TPtr<T> AsPtr() const
		{
			return Cast<T>(ownership.AsPtr());
		}
		template<typename T = Object>
		TPtr<T> GetOwner() const
		{
			return Cast<T>(ownership.GetOwner());
		}
	};


	template<typename T>
	concept IsObject = Derived<T, BaseObject, false>;
}    // namespace p