// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "BaseClass.h"
#include "ClassPtrBuilder.h"
#include "Pipe/Core/Log.h"
#include "Pipe/Reflect/Reflection.h"
#include "Pipe/Serialize/Serialization.h"


namespace p
{
	class Context;


	// For shared export purposes, we separate pointers from the exported Class
	struct PIPE_API ClassOwnership
	{
		TPtr<BaseClass> self;
		TPtr<BaseClass> owner;
		static TPtr<BaseClass> nextOwner;


		ClassOwnership() : owner{Move(nextOwner)}, self{} {}

		const TPtr<BaseClass>& GetSelf() const
		{
			return self;
		}
		const TPtr<BaseClass>& GetOwner() const
		{
			return owner;
		}
	};


	class PIPE_API Class : public BaseClass
	{
	public:
		using Super       = BaseClass;
		using BuilderType = p::TClassTypeBuilder<Class, void, Type_NoFlag>;


		static p::ClassType* GetStaticType()
		{
			return p::GetType<Class>();
		}
		static constexpr TypeFlags GetStaticFlags()
		{
			return Type_NoFlag;
		}
		virtual p::ClassType* GetType() const
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

		void ChangeOwner(const TPtr<BaseClass>& inOwner)
		{
			ownership.owner = inOwner;
		}

		virtual void Serialize(ReadWriter& ct)
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


}    // namespace p


template<typename T>
void Read(p::Reader& ct, T& value) requires(p::Derived<T, p::Class>)
{
	ct.BeginObject();
	p::ReadWriter common{ct};
	value.Serialize(common);
}

template<typename T>
void Write(p::Writer& ct, const T& value) requires(p::Derived<T, p::Class>)
{
	ct.BeginObject();
	p::ReadWriter common{ct};
	const_cast<T&>(value).Serialize(common);
}
