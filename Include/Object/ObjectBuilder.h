// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Log.h"
#include "Reflection/ReflectionTraits.h"
#include "Reflection/Static/ClassType.h"
#include "Reflection/TypeName.h"


namespace Rift
{
	template <typename T>
	struct ObjectBuilder
	{
		static TOwnPtr<T, ObjectBuilder<T>> New(
		    Refl::ClassType* objectClass, TPtr<BaseObject> owner = {})
		{
			static_assert(IsClass<T>(), "Type is not an Object!");
			if (objectClass)
			{
				return objectClass->CreateInstance(owner).Cast<T>();
			}
			return {};
		}

		static TOwnPtr<T, ObjectBuilder<T>> New(TPtr<BaseObject> owner = {})
		{
			static_assert(IsClass<T>(), "Type is not an Object!");

			if constexpr (std::is_abstract_v<T>)
			{
				Log::Error(
				    "Tried to create an instance of '{}' which is abstract.", GetTypeName<T>());
				return {};
			}
			else
			{
				TOwnPtr<T, ObjectBuilder<T>> ptr{new T()};
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
