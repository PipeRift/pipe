// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Log.h"
#include "Reflection/ReflectionTypeTraits.h"
#include "Reflection/Static/ClassType.h"


namespace Rift
{
	template <typename T>
	struct ObjectBuilder
	{
		static OwnPtr<T, ObjectBuilder<T>> New(
		    Refl::ClassType* objectClass, Ptr<BaseObject> owner = {})
		{
			static_assert(IsClass<T>(), "Type is not an Object!");
			if (objectClass)
			{
				return objectClass->CreateInstance(owner).Cast<T>();
			}
			return {};
		}

		static OwnPtr<T, ObjectBuilder<T>> New(Ptr<BaseObject> owner = {})
		{
			static_assert(IsClass<T>(), "Type is not an Object!");

			if constexpr (std::is_abstract_v<T>)
			{
				Log::Error("Tried to create an instance of '{}' which is abstract.",
				    GetReflectedName<T>().ToString());
				return {};
			}
			else
			{
				OwnPtr<T, ObjectBuilder<T>> ptr{new T()};
				ptr->PreConstruct(ptr.AsPtr(), owner);
				ptr->Construct();
				return ptr;
			}
		}

		static void Delete(void* rawPtr)
		{
			T* ptr = static_cast<T*>(rawPtr);
			ptr->StartDestroy();
			delete ptr;
		}
	};
}    // namespace Rift
