// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Static/DataType.h"


namespace Rift::Refl
{
	class StructType : public DataType
	{
	public:
		// NOTE: Most of the class comparison functions do actually
		// call Type to reduce complexity and code duplication.
		//
		// We can cast safely to Type since Structs only inherit Structs

		CORE_API StructType* GetParent() const
		{
			return static_cast<StructType*>(parent);
		}

		CORE_API const TArray<StructType*>& GetChildren() const
		{
			// Structs only have Struct children. It is safe to reinterpret_cast.
			return reinterpret_cast<const TArray<StructType*>&>(DataType::GetChildren());
		}

		CORE_API void GetChildrenDeep(TArray<StructType*>& outChildren) const
		{
			DataType::GetChildrenDeep(reinterpret_cast<TArray<DataType*>&>(outChildren));
		}

		CORE_API StructType* FindChild(const Name& Name) const
		{
			return static_cast<StructType*>(DataType::FindChild(Name));
		}

		CORE_API bool IsA(StructType* other) const
		{
			return this == other;
		}
	};
}    // namespace Rift::Refl
