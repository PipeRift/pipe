// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "PRefl/Builders/StaticInitializers.h"
#include "PRefl/ReflectionTraits.h"
#include "PRefl/Type.h"


namespace p
{
	template<typename T>
	struct TTypeInstance
	{
		static Type* instance;

		static Type* InitType()
		{
			if constexpr (IsClass<T>() || IsStruct<T>())
			{
				instance = T::InitType();
			}
			else if constexpr (IsReflectedEnum<T>())
			{
				instance = reflection::TStaticEnumInitializer<T>::onInit();
			}
			else if constexpr (IsReflectedNative<T>())
			{
				instance = reflection::TStaticNativeInitializer<T>::onInit();
			}
			return instance;
		}

		static Type* GetType()
		{
			return instance;
		}
	};

	template<typename T>
	inline Type* TTypeInstance<T>::instance = TTypeInstance<T>::InitType();
}    // namespace p
