// Copyright 2015-2021 Piperift - All rights reserved
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

		CORE_API void GetAllChildren(TArray<StructType*>& outChildren)
		{
			__GetAllChildren(reinterpret_cast<TArray<DataType*>&>(outChildren));
		}

		CORE_API StructType* FindChild(const Name& Name) const
		{
			return static_cast<StructType*>(__FindChild(Name));
		}

		CORE_API bool IsA(StructType* other) const
		{
			return this == other;
		}
	};
}    // namespace Rift::Refl
