// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Events/Function.h"
#include "Reflection/Static/DataType.h"
#include "Types/BaseClass.h"


namespace Rift
{
	template<typename T>
	struct ClassBuilder;
}

namespace Rift::Refl
{
	class ClassType : public DataType
	{
		template<typename T, typename Parent, TypeFlags flags, typename TType>
		friend struct TClassTypeBuilder;

		using CreateFunc = TFunction<BaseClass*()>;

		CreateFunc onCreate;


	public:
		CORE_API BaseClass* CreateInstance();

		CORE_API ClassType* GetParent() const
		{
			return static_cast<ClassType*>(parent);
		}

		CORE_API void GetAllChildren(TArray<ClassType*>& outChildren)
		{
			// Classes only have Class children. It is safe to reinterpret_cast.
			__GetAllChildren(reinterpret_cast<TArray<DataType*>&>(outChildren));
		}

		CORE_API ClassType* FindChild(const Name& className) const
		{
			// Classes only have Class children. It is safe to static_cast.
			return static_cast<ClassType*>(__FindChild(className));
		}

		CORE_API bool IsA(ClassType* other) const
		{
			return this == other;
		}
	};
}    // namespace Rift::Refl
