// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Log.h"
#include "Memory/PtrBuilder.h"
#include "Reflection/ClassType.h"
#include "Reflection/ReflectionTraits.h"
#include "Reflection/TypeName.h"



namespace Rift
{
	template<typename T>
	struct TClassPtrBuilder : public TPtrBuilder<T>
	{
		template<typename... Args>
		static T* New(Args&&... args, const TPtr<BaseClass>& owner = {})
		{
			T* instance = new T(std::forward<Args>(args)...);
			instance->SetOwner(owner);
			return instance;
		}

		// Allow creation of classes using reflection
		static T* New(Refl::ClassType* type, TPtr<BaseClass> owner = {})
		{
			if (type)
			{
				if (T* instance = dynamic_cast<T*>(type->New()))
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
