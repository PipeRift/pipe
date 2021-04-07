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

	class CORE_API Object : public BaseObject
	{
		ORPHAN_CLASS(Object, ReflectionTags::None)

	private:
		PROP(Name, name);
		Name name;

		TPtr<BaseObject> self;
		TPtr<BaseObject> owner;


	public:
		Object() : BaseObject(), self{}, owner{} {};

		void PreConstruct(TPtr<BaseObject>&& inSelf, const TPtr<BaseObject>& inOwner)
		{
			self  = inSelf;
			owner = inOwner;
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
			return self.Cast<T>();
		}

		template <typename T = Object>
		TPtr<T> GetOwner() const
		{
			return owner.Cast<T>();
		}

		void SetName(Name newName)
		{
			name = Move(newName);
		}
		Name GetName() const
		{
			return name;
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
