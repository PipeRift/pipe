// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Events/Function.h"
#include "Memory/OwnPtr.h"
#include "Object/BaseObject.h"
#include "Reflection/Static/Type.h"


namespace Rift
{
	template <typename T>
	struct ObjectBuilder;
}

namespace Rift::Refl
{
	class CORE_API ClassType : public DataType
	{
		template <typename T, typename Parent, ReflectionTags tags>
		friend struct TClassTypeBuilder;

		using CreateFunc =
		    TFunction<OwnPtr<BaseObject, ObjectBuilder<BaseObject>>(const Ptr<BaseObject>&)>;

		CreateFunc onCreate;


	public:
		OwnPtr<BaseObject, ObjectBuilder<BaseObject>> CreateInstance(const Ptr<BaseObject>& owner);

		ClassType* GetParent() const
		{
			return static_cast<ClassType*>(parent);
		}

		void GetAllChildren(TArray<ClassType*>& outChildren)
		{
			// Classes only have Class children. It is safe to reinterpret_cast.
			__GetAllChildren(reinterpret_cast<TArray<DataType*>&>(outChildren));
		}

		ClassType* FindChild(const Name& className) const
		{
			// Classes only have Class children. It is safe to static_cast.
			return static_cast<ClassType*>(__FindChild(className));
		}

		bool IsA(ClassType* other) const
		{
			return this == other;
		}
	};
}    // namespace Rift::Refl
