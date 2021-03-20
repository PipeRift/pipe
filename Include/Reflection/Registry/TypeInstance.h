// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Registry/StaticInitializers.h"
#include "Reflection/Static/Type.h"


namespace Rift::Refl
{
	template <typename T>
	struct TTypeInstance
	{
		static Type* instance;


		static Type* InitType() requires(IsClass<T>() || IsStruct<T>())
		{
			return T::InitType();
		}

		static Type* InitType() requires(IsEnum<T>())
		{
			return TStaticEnumInitializer<T>::onInit();
		}

		static Type* InitType() requires(IsNative<T>())
		{
			return TStaticNativeInitializer<T>::onInit();
		}
	};

	template <typename T>
	inline Type* TTypeInstance<T>::instance = TTypeInstance<T>::InitType();


	template <typename T>
	Refl::Type* InternalGetType()
	{
		return TTypeInstance<T>::instance;
	}
}    // namespace Rift::Refl
