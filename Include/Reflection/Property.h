// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Memory/OwnPtr.h"
#include "Reflection/ReflectionFlags.h"
#include "Strings/Name.h"
#include "Strings/String.h"


namespace Rift::Refl
{
	class DataType;

	/**
	 * Static information about a property
	 */
	class Property
	{
		friend class ReflectionRegistry;

	public:
		// Returns a pointer to the variable from an owner instance pointer
		using Access = void*(void*);

	protected:
		DataType* owner = nullptr;
		Type* type      = nullptr;
		Name name;

		Access* access  = nullptr;
		PropFlags flags = Prop_NoFlag;
		String displayName;


	public:
		Property()                = delete;
		Property(Property&&)      = delete;
		Property(const Property&) = delete;

	protected:
		CORE_API Property(DataType* owner, Type* type, Name name, Access* access, PropFlags flags)
		    : owner(owner), type(type), name(name), access(access), flags(flags)
		{
			displayName = Strings::ToSentenceCase(name.ToString());
		}

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
}    // namespace Rift::Refl
