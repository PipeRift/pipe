// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "BaseClass.h"
#include "ClassPtrBuilder.h"
#include "Log.h"
#include "Reflection/Reflection.h"
#include "Serialization/Contexts.h"


namespace p
{
	class Context;
}

namespace p::refl
{
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
		using Super       = BaseClass;
		using BuilderType = p::refl::TClassTypeBuilder<Class, void, Type_NoFlag>;


		static p::refl::ClassType* GetStaticType()
		{
			return p::GetType<Class>();
		}
		static constexpr TypeFlags GetStaticFlags()
		{
			return Type_NoFlag;
		}
		virtual p::refl::ClassType* GetType() const
		{
			return p::GetType<Class>();
		}

		REFLECTION_BODY({})

	public:
		template<typename T>
		using PtrBuilder = TClassPtrBuilder<T>;


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

		virtual void Serialize(serl::CommonContext& ct)
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


}    // namespace p::refl

namespace p
{
	using namespace p::refl;
}

template<typename T>
void Read(p::ReadContext& ct, T& value) requires(p::Derived<T, p::Class>)
{
	ct.BeginObject();
	p::CommonContext common{ct};
	value.Serialize(common);
}

template<typename T>
void Write(p::WriteContext& ct, const T& value) requires(p::Derived<T, p::Class>)
{
	ct.BeginObject();
	p::CommonContext common{ct};
	const_cast<T&>(value).Serialize(common);
}
