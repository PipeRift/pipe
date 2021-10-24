// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Log.h"
#include "Memory/PtrBuilder.h"
#include "Reflection/ReflectionTraits.h"
#include "Reflection/Static/ClassType.h"
#include "Reflection/TypeName.h"


namespace Rift
{
	template<typename T>
	struct TObjectBuilder : public TPtrBuilder<T>
	{
		template<typename... Args>
		static T* New(Args&&... args, const TPtr<BaseObject>& owner = {})
		{
			T* instance = new T(std::forward<Args>(args)...);
			instance->SetOwner(owner);
			return instance;
		}

		// Allow creation of classes using reflection
		static T* New(Refl::ClassType* objClass, TPtr<BaseObject> owner = {})
		{
			if (objClass)
			{
				if (T* instance = dynamic_cast<T*>(objClass->CreateInstance()))
				{
					instance->SetOwner(owner);
					return instance;
				}
			}
			return {};
		}

		static void PostNew(TOwnPtr<T>& ptr)
		{
			ptr->PreConstruct(ptr);
			ptr->Construct();
		}

		static void Delete(void* rawPtr)
		{
			T* ptr = static_cast<T*>(rawPtr);
			ptr->StartDestroy();
			delete ptr;
		}

		static T* NewArray(sizet size)                        = delete;
		static void PostNewArray(TOwnPtr<T>& ptr, sizet size) = delete;
	};
}    // namespace Rift
