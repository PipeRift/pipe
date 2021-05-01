// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Events/Function.h"
#include "Memory/OwnPtr.h"
#include "Object/BaseObject.h"
#include "Reflection/Static/DataType.h"


namespace Rift
{
	template <typename T>
	struct ObjectBuilder;
}

namespace Rift::Refl
{
	class ClassType : public DataType
	{
		template <typename T, typename Parent, ReflectionTags tags>
		friend struct TClassTypeBuilder;

		using CreateFunc =
		    TFunction<TOwnPtr<BaseObject, ObjectBuilder<BaseObject>>(const TPtr<BaseObject>&)>;

		CreateFunc onCreate;


	public:
		CORE_API TOwnPtr<BaseObject, ObjectBuilder<BaseObject>> CreateInstance(
		    const TPtr<BaseObject>& owner);

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
