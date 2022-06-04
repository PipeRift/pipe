// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Memory/OwnPtr.h"
#include "Reflection/ReflectionFlags.h"
#include "Reflection/Type.h"
#include "Strings/Name.h"
#include "Strings/String.h"


namespace Pipe::Refl
{
	class DataType;

	/**
	 * Static information about a property
	 */
	class Property
	{
		template<typename T, typename Parent, TypeFlags flags, typename TType>
		friend struct TDataTypeBuilder;

	public:
		// Returns a pointer to the variable from an owner instance pointer
		using AccessFunc = void*(void*);

	protected:
		DataType* owner = nullptr;
		Type* type      = nullptr;
		Name name;

		AccessFunc* access = nullptr;
		PropFlags flags    = Prop_NoFlag;
		String displayName;


	public:
		CORE_API Property()       = default;
		Property(Property&&)      = delete;
		Property(const Property&) = delete;

	public:
		CORE_API virtual ~Property() = default;

		CORE_API DataType* GetOwner() const;
		CORE_API Type* GetType() const;
		CORE_API Name GetName() const;
		CORE_API void* GetDataPtr(void* container) const;
		CORE_API StringView GetDisplayName() const;
		CORE_API bool HasFlag(PropFlags flag) const;
		CORE_API bool HasAllFlags(PropFlags inFlags) const;
		CORE_API bool HasAnyFlags(PropFlags inFlags) const;
	};


	inline DataType* Property::GetOwner() const
	{
		return owner;
	}
	inline Type* Property::GetType() const
	{
		return type;
	}
	inline Name Property::GetName() const
	{
		return name;
	}
	inline void* Property::GetDataPtr(void* container) const
	{
		return access ? access(container) : nullptr;
	}
	inline StringView Property::GetDisplayName() const
	{
		return displayName;
	}
}    // namespace Pipe::Refl
