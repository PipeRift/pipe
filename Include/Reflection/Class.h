// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "BaseClass.h"
#include "ClassPtrBuilder.h"
#include "Log.h"
#include "Reflection/Reflection.h"
#include "Serialization/Contexts.h"


namespace Pipe
{
	class Context;
}

namespace Pipe::Refl
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
		using BuilderType = Pipe::Refl::TClassTypeBuilder<Class, void, Type_NoFlag>;


		static Pipe::Refl::ClassType* GetStaticType()
		{
			return Pipe::GetType<Class>();
		}
		static constexpr TypeFlags GetStaticFlags()
		{
			return Type_NoFlag;
		}
		virtual Pipe::Refl::ClassType* GetType() const
		{
			return Pipe::GetType<Class>();
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


}    // namespace Pipe::Refl

namespace Pipe
{
	using namespace Pipe::Refl;
}

template<typename T>
void Read(Pipe::ReadContext& ct, T& value) requires(Pipe::Derived<T, Pipe::Class>)
{
	ct.BeginObject();
	Pipe::CommonContext common{ct};
	value.Serialize(common);
}

template<typename T>
void Write(Pipe::WriteContext& ct, const T& value) requires(Pipe::Derived<T, Pipe::Class>)
{
	ct.BeginObject();
	Pipe::CommonContext common{ct};
	const_cast<T&>(value).Serialize(common);
}
