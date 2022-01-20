// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "BaseClass.h"
#include "ClassBuilder.h"
#include "Log.h"
#include "Reflection/Reflection.h"
#include "Serialization/Contexts.h"


namespace Rift
{
	class Context;


	// For shared export purposes, we separate pointers from the exported Class
	struct ClassOwnership
	{
		TPtr<BaseClass> self;
		TPtr<BaseClass> owner;

		CORE_API const TPtr<BaseClass>& GetSelf() const
		{
			return self;
		}
		CORE_API const TPtr<BaseClass>& GetOwner() const
		{
			return owner;
		}
	};


	class CORE_API Class : public BaseClass
	{
	public:
		using BuilderType = Rift::Refl::TClassTypeBuilder<Class, void, Type_NoFlag>;

		virtual Rift::Refl::ClassType* GetClass() const
		{
			return Rift::GetType<Class>();
		}
		virtual void SerializeReflection(Rift::Serl::CommonContext& ct)
		{
			__ReflSerializeProperty(ct, Rift::Refl::MetaCounter<0>{});
		}

		CLASS_BODY(Class, {})

	public:
		template<typename T>
		using PtrBuilder = TClassBuilder<T>;


	private:
		ClassOwnership ownership;


	public:
		Class() = default;

		void SetOwner(const TPtr<BaseClass>& inOwner)
		{
			ownership.owner = inOwner;
		}

		void PreConstruct(TPtr<BaseClass>&& inSelf)
		{
			ownership.self = inSelf;
		}
		virtual void Construct() {}

		virtual void Serialize(Serl::CommonContext& ct)
		{
			SerializeReflection(ct);
		}

		template<typename T = Class>
		TPtr<T> Self() const
		{
			return ownership.GetSelf().Cast<T>();
		}

		template<typename T = Class>
		TPtr<T> GetOwner() const
		{
			return ownership.GetOwner().Cast<T>();
		}
	};


	template<typename T>
	void Read(Serl::ReadContext& ct, T& value) requires(Derived<T, Class>)
	{
		ct.BeginObject();
		Serl::CommonContext common{ct};
		value.Serialize(common);
	}

	template<typename T>
	void Write(Serl::WriteContext& ct, const T& value) requires(Derived<T, Class>)
	{
		ct.BeginObject();
		Serl::CommonContext common{ct};
		const_cast<T&>(value).Serialize(common);
	}
}    // namespace Rift
