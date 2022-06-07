// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Reflect/Builders/StaticInitializers.h"
#include "Pipe/Reflect/ReflectionTraits.h"
#include "Pipe/Reflect/Type.h"


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
