// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Events/Function.h"
#include "Memory/OwnPtr.h"
#include "Object/BaseObject.h"
#include "Object/ObjectBuilder.h"
#include "Reflection/Static/Type.h"


namespace Rift
{
	template <typename T>
	struct ObjectBuilder;
}

namespace Rift::Refl
{
	class CORE_API Class : public Type
	{
		template <typename T, typename Parent, ReflectionTags tags>
		friend struct TClassBuilder;

		using CreateFunc =
		    TFunction<OwnPtr<BaseObject, ObjectBuilder<BaseObject>>(const Ptr<BaseObject>&)>;

		CreateFunc onCreate;


	public:
		OwnPtr<BaseObject, ObjectBuilder<BaseObject>> CreateInstance(const Ptr<BaseObject>& owner)
		{
			if (onCreate)
			{
				return onCreate(owner);
			}
			return {};
		}

		Class* GetParent() const
		{
			return static_cast<Class*>(parent);
		}

		void GetAllChildren(TArray<Class*>& outChildren)
		{
			// Classes only have Class children. It is safe to reinterpret_cast.
			__GetAllChildren(reinterpret_cast<TArray<Type*>&>(outChildren));
		}

		Class* FindChild(const Name& className) const
		{
			// Classes only have Class children. It is safe to static_cast.
			return static_cast<Class*>(__FindChild(className));
		}

		bool IsA(Class* other) const
		{
			return this == other;
		}
	};
}    // namespace Rift::Refl
