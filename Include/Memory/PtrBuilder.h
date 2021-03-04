// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Object/BaseObject.h"
#include "Reflection/ReflectionTypeTraits.h"
#include "TypeTraits.h"


namespace Rift
{
	template <typename T>
	struct PtrBuilder
	{
		template <typename... Args>
		static T* New(Args&&... args)
		{
			return new T(std::forward<Args>(args)...);
		}

		static T* NewArray(sizet size)
		{
			using Elem = std::remove_extent_t<T>;
			return new Elem[size]();
		}

		static void Delete(void* rawPtr)
		{
			T* ptr = static_cast<T*>(rawPtr);
			if constexpr (IsObject<T>())
			{
				ptr->StartDestroy();
			}
			delete ptr;
		}
	};
}    // namespace Rift
