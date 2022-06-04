// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Builders/StaticInitializers.h"
#include "Reflection/ReflectionTraits.h"
#include "Reflection/Type.h"


namespace Pipe::Refl
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
				instance = TStaticEnumInitializer<T>::onInit();
			}
			else if constexpr (IsReflectedNative<T>())
			{
				instance = TStaticNativeInitializer<T>::onInit();
			}
			return instance;
		}

		static Refl::Type* GetType()
		{
			return instance;
		}
	};

	template<typename T>
	inline Type* TTypeInstance<T>::instance = TTypeInstance<T>::InitType();
}    // namespace Pipe::Refl
