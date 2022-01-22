// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Registry/StaticInitializers.h"
#include "Reflection/Static/Type.h"


namespace Rift::Refl
{
	template<typename T>
	struct TTypeInstance
	{
		static Type* instance;


		static Type* InitType()
		{
			if constexpr (IsClass<T>() || IsStruct<T>())
			{
				return T::InitType();
			}
			else if constexpr (IsReflectedEnum<T>())
			{
				return TStaticEnumInitializer<T>::onInit();
			}
			else if constexpr (IsReflectedNative<T>())
			{
				return TStaticNativeInitializer<T>::onInit();
			}
			return nullptr;
		}
	};

	template<typename T>
	inline Type* TTypeInstance<T>::instance = TTypeInstance<T>::InitType();


	template<typename T>
	Refl::Type* InternalGetType()
	{
		return TTypeInstance<T>::instance;
	}
}    // namespace Rift::Refl
