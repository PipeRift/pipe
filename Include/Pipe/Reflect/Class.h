// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "BaseClass.h"
#include "ClassPtrBuilder.h"
#include "Pipe/Reflect/Reflection.h"
#include "PipeSerialize.h"


namespace p
{
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

		P_REFLECTION_BODY({})

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
