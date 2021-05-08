// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "BaseObject.h"
#include "Log.h"
#include "ObjectBuilder.h"
#include "Reflection/Static/ClassType.h"
#include "Serialization/Archive.h"


namespace Rift
{
	class Context;


	// For shared export purposes, we separate pointers from the exported Object
	struct ObjectOwnership
	{
		TPtr<BaseObject> self;
		TPtr<BaseObject> owner;

		CORE_API const TPtr<BaseObject>& GetSelf() const
		{
			return self;
		}
		CORE_API const TPtr<BaseObject>& GetOwner() const
		{
			return owner;
		}
	};


	class CORE_API Object : public BaseObject
	{
		ORPHAN_CLASS(Object, ReflectionTags::None)

	public:
		template <typename T>
		using PtrBuilder = TObjectBuilder<T>;


	private:
		ObjectOwnership ownership;


	public:
		Object() = default;

		void SetOwner(const TPtr<BaseObject>& inOwner)
		{
			ownership.owner = inOwner;
		}

		void PreConstruct(TPtr<BaseObject>&& inSelf)
		{
			ownership.self = inSelf;
		}
		virtual void Construct() {}

		virtual bool Serialize(Archive& ar)
		{
			SerializeReflection(ar);
			return true;
		}

		template <typename T = Object>
		TPtr<T> Self() const
		{
			return ownership.GetSelf().Cast<T>();
		}

		template <typename T = Object>
		TPtr<T> GetOwner() const
		{
			return ownership.GetOwner().Cast<T>();
		}

		virtual TPtr<Context> GetContext() const;
	};


	template <class T>
	static TOwnPtr<T> Create(Refl::ClassType* objectClass, const TPtr<Object> owner = {})
	{
		return MakeOwned<T>(objectClass, owner);
	}

	template <class T>
	static TOwnPtr<T> Create(const TPtr<Object> owner = {})
	{
		return MakeOwned<T>(owner);
	}
	// END - Pointer helpers
}    // namespace Rift
