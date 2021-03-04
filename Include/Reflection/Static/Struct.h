// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Static/TProperty.h"
#include "Type.h"


namespace Rift::Refl
{
	class CORE_API Struct : public Type
	{
	public:
		// NOTE: Most of the class comparison functions do actually
		// call Type to reduce complexity and code duplication.
		//
		// We can cast safely to Type since Structs only inherit Structs

		Struct* GetParent() const
		{
			return static_cast<Struct*>(parent);
		}

		void GetAllChildren(TArray<Struct*>& outChildren)
		{
			__GetAllChildren(reinterpret_cast<TArray<Type*>&>(outChildren));
		}

		Struct* FindChild(const Name& Name) const
		{
			return static_cast<Struct*>(__FindChild(Name));
		}

		bool IsA(Struct* other) const
		{
			return this == other;
		}
	};
}    // namespace Rift::Refl
