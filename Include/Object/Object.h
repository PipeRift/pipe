// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "BaseObject.h"
#include "Log.h"
#include "ObjectBuilder.h"
#include "Reflection/Reflection.h"
#include "Serialization/Contexts.h"


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
	public:
		using BuilderType = Rift::Refl::TClassTypeBuilder<Object, void, ReflectionTags::None>;

		virtual Rift::Refl::ClassType* GetClass() const
		{
			return Rift::GetType<Object>();
		}
		virtual void SerializeReflection(Rift::Serl::CommonContext& ct)
		{
			__ReflSerializeProperty(ct, Rift::Refl::MetaCounter<0>{});
		}

		CLASS_BODY(Object, {})

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

		virtual void Serialize(Serl::CommonContext& ct)
		{
			SerializeReflection(ct);
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
	};


	template <typename T>
	static TOwnPtr<T> Create(Refl::ClassType* objectClass, const TPtr<Object> owner = {}) requires(
	    Derived<T, Object>)
	{
		return MakeOwned<T>(objectClass, owner);
	}

	template <typename T>
	static TOwnPtr<T> Create(const TPtr<Object> owner = {}) requires(Derived<T, Object>)
	{
		return MakeOwned<T>(owner);
	}
	// END - Pointer helpers


	template <typename T>
	void Read(Serl::ReadContext& ct, T& value) requires(Derived<T, Object>)
	{
		ct.BeginObject();
		Serl::CommonContext common{ct};
		value.Serialize(common);
	}

	template <typename T>
	void Write(Serl::WriteContext& ct, const T& value) requires(Derived<T, Object>)
	{
		ct.BeginObject();
		Serl::CommonContext common{ct};
		const_cast<T&>(value).Serialize(common);
	}
}    // namespace Rift
