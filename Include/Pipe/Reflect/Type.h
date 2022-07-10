// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/EnumFlags.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Reflect/TypeId.h"


namespace p
{
	enum class TypeCategory : u8
	{
		None   = 0,
		Native = 1 << 0,
		Enum   = 1 << 1,
		Data   = 1 << 2,
		Struct = 1 << 3,
		Class  = 1 << 4,
		All    = Native | Enum | Data | Struct | Class
	};
	PIPE_DEFINE_FLAG_OPERATORS(TypeCategory)


	/** Smallest reflection type */
	class PIPE_API Type
	{
	protected:
		TypeId id;
		TypeCategory category = TypeCategory::None;
		sizet size            = 0;

#pragma warning(push)
#pragma warning(disable:4251)
		StringView name;
#pragma warning(pop)


	protected:
		Type(TypeCategory category) : category{category} {}

	public:
		Type(const Type&) = delete;
		Type& operator=(const Type&) = delete;
		virtual ~Type()              = default;

		TypeId GetId() const
		{
			return id;
		}
		StringView GetName() const;

		TypeCategory GetCategory() const
		{
			return category;
		}
		sizet GetSize() const
		{
			return size;
		}

		class NativeType* AsNative();
		class EnumType* AsEnum();
		class DataType* AsData();
		class StructType* AsStruct();
		class ClassType* AsClass();
	};
}    // namespace p
