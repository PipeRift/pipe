// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Hash.h"
#include "Pipe/Reflect/Reflection.h"
#include "Pipe/Reflect/Struct.h"
#include "Pipe/Serialize/Serialization.h"


namespace p
{
	struct PIPE_API BaseSubType : public Struct
	{
		STRUCT(BaseSubType, Struct)

	protected:
		const Type* type = nullptr;

	public:
		bool Is(const Type* other) const
		{
			return type == other;
		}

		const Type* Get() const
		{
			return type;
		}

		bool operator==(const Type* other) const
		{
			return Is(other);
		}
		bool operator==(const BaseSubType& other) const
		{
			return Is(other.type);
		}
		const Type& operator*() const
		{
			return *type;
		}
		const Type* operator->() const
		{
			return type;
		}
	};

	struct PIPE_API SubType : public BaseSubType
	{
		STRUCT(SubType, BaseSubType)

		const Type* base = nullptr;


		SubType(const Type* base, const Type* type = nullptr) : base{base}
		{
			Set(type);
		}

		bool Set(const Type* newType);
		void SetBase(const Type* newBase);
	};

	template<typename T>
	requires(IsStruct<T>() || IsClass<T>())
	struct TSubType : public BaseSubType
	{
		TSubType() = default;
		TSubType(const Type* type)
		{
			Set(type);
		}

		bool Set(const Type* newType)
		{
			if (!newType || T::GetStaticType()->IsParentOf(Cast<DataType>(newType)))
			{
				type = newType;
				return true;
			}
			return false;
		}
	};

	template<>
	struct Hash<BaseSubType> : public Hash<Type*>
	{
		sizet operator()(const BaseSubType& type) const
		{
			return reinterpret_cast<sizet>(type.Get());
		}
	};
}    // namespace p
