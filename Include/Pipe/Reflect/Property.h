// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/String.h"
#include "Pipe/Core/Tag.h"
#include "Pipe/Reflect/ReflectionFlags.h"
#include "Pipe/Reflect/Type.h"


namespace p
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
		Tag name;

		AccessFunc* access = nullptr;
		PropFlags flags    = Prop_NoFlag;
		String displayName;


	public:
		PIPE_API Property()       = default;
		Property(Property&&)      = delete;
		Property(const Property&) = delete;

	public:
		PIPE_API virtual ~Property() = default;

		PIPE_API DataType* GetOwner() const;
		PIPE_API Type* GetType() const;
		PIPE_API Tag GetName() const;
		PIPE_API void* GetPtrInContainer(void* container) const;
		PIPE_API StringView GetDisplayName() const;
		PIPE_API bool HasFlag(PropFlags flag) const;
		PIPE_API bool HasAllFlags(PropFlags inFlags) const;
		PIPE_API bool HasAnyFlags(PropFlags inFlags) const;
	};


	inline DataType* Property::GetOwner() const
	{
		return owner;
	}
	inline Type* Property::GetType() const
	{
		return type;
	}
	inline Tag Property::GetName() const
	{
		return name;
	}
	inline void* Property::GetPtrInContainer(void* container) const
	{
		return access ? access(container) : nullptr;
	}
	inline StringView Property::GetDisplayName() const
	{
		return displayName;
	}
}    // namespace p
