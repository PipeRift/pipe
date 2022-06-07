// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Log.h"
#include "Pipe/Memory/PtrBuilder.h"
#include "Pipe/Reflect/ClassType.h"
#include "Pipe/Reflect/ReflectionTraits.h"
#include "Pipe/Reflect/TypeName.h"


namespace p
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
		static T* New(ClassType* type, TPtr<BaseClass> owner = {})
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
}    // namespace p
