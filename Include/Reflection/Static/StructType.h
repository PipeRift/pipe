// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Static/DataType.h"
#include "Reflection/Static/TProperty.h"


namespace Rift::Refl
{
	class CORE_API StructType : public DataType
	{
	public:
		// NOTE: Most of the class comparison functions do actually
		// call Type to reduce complexity and code duplication.
		//
		// We can cast safely to Type since Structs only inherit Structs

		StructType* GetParent() const
		{
			return static_cast<StructType*>(parent);
		}

		void GetAllChildren(TArray<StructType*>& outChildren)
		{
			__GetAllChildren(reinterpret_cast<TArray<DataType*>&>(outChildren));
		}

		StructType* FindChild(const Name& Name) const
		{
			return static_cast<StructType*>(__FindChild(Name));
		}

		bool IsA(StructType* other) const
		{
			return this == other;
		}
	};
}    // namespace Rift::Refl
