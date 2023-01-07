// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Reflect/DataType.h"


namespace p
{
	class StructType : public DataType
	{
	public:
		static constexpr TypeCategory typeCategory = TypeCategory::Struct;


	public:
		StructType() : DataType(typeCategory) {}

		// NOTE: Most of the class comparison functions do actually
		// call Type to reduce complexity and code duplication.
		//
		// We can cast safely to Type since Structs only inherit Structs

		PIPE_API StructType* GetParent() const
		{
			return static_cast<StructType*>(parent);
		}

		PIPE_API const TArray<StructType*>& GetChildren() const
		{
			// Structs only have Struct children. It is safe to reinterpret_cast.
			return reinterpret_cast<const TArray<StructType*>&>(DataType::GetChildren());
		}

		PIPE_API void GetChildrenDeep(TArray<StructType*>& outChildren) const
		{
			DataType::GetChildrenDeep(reinterpret_cast<TArray<DataType*>&>(outChildren));
		}

		PIPE_API StructType* FindChild(const Name& Name) const
		{
			return static_cast<StructType*>(DataType::FindChild(Name));
		}

		PIPE_API bool IsA(StructType* other) const
		{
			return this == other;
		}
	};
}    // namespace p
