// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Log.h"
#include "Pipe/Memory/PtrBuilder.h"
#include "Pipe/Reflect/BaseClass.h"
#include "Pipe/Reflect/ClassType.h"
#include "Pipe/Reflect/ReflectionTraits.h"
#include "Pipe/Reflect/TypeName.h"


namespace p
{
	template<typename T>
	struct TClassPtrBuilder : public TPtrBuilder<T>
	{
		template<typename... Args>
		static T* New(Arena& arena, Args&&... args, const TPtr<BaseClass>& owner = {})
		{
			// Sets owner during construction
			// TODO: Fix self not existing at the moment of construction
			ClassOwnership::nextOwner = owner;
			return new (p::Alloc<T>(arena)) T(std::forward<Args>(args)...);
		}

		// Allow creation of classes using reflection
		static T* New(Arena& arena, ClassType* type, TPtr<BaseClass> owner = {})
		{
			if (type)
			{
				// Sets owner during construction
				// TODO: Fix self not existing at the moment of construction
				ClassOwnership::nextOwner = owner;
				if (T* instance = dynamic_cast<T*>(type->New(arena)))
				{
					return instance;
				}
			}
			return {};
		}


		static void Delete(Arena& arena, void* rawPtr)
		{
			T* ptr               = static_cast<T*>(rawPtr);
			const sizet typeSize = ptr->GetType()->GetSize();
			ptr->~T();
			p::Free(arena, (void*)ptr, typeSize);
		}
	};
}    // namespace p
