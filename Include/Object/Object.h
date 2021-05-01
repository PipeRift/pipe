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

	private:
		ObjectOwnership ownership;


	public:
		Object() = default;

		void PreConstruct(TPtr<BaseObject>&& inSelf, const TPtr<BaseObject>& inOwner)
		{
			ownership.self  = inSelf;
			ownership.owner = inOwner;
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


	// BEGIN - Pointer helpers
	template <typename T>
	using ObjectPtr = TOwnPtr<T, ObjectBuilder<T>>;

	template <class T>
	static ObjectPtr<T> Create(Refl::ClassType* objectClass, const TPtr<Object> owner = {})
	{
		return MakeOwned<T, ObjectBuilder<T>>(objectClass, owner);
	}

	template <class T>
	static ObjectPtr<T> Create(const TPtr<Object> owner = {})
	{
		return MakeOwned<T, ObjectBuilder<T>>(owner);
	}
	// END - Pointer helpers
}    // namespace Rift
