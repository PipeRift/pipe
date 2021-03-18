// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Static/Type.h"


namespace Rift::Refl
{
	template <typename T>
	struct TTypeInstance
	{
		static Type* instance;


		static DataType* InitType() requires(IsClass<T>() || IsStruct<T>())
		{
			return T::InitType();
		}

		static EnumType* InitType() requires(IsEnum<T>())
		{
			return TStaticEnumInitializer<T>::onInit();
		}
	};

	template <typename T>
	inline Type* TTypeInstance<T>::instance = TTypeInstance<T>::InitType();
}    // namespace Rift::Refl
