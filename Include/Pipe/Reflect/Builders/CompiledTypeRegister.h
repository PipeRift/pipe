// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Reflect/Builders/StaticInitializers.h"
#include "Pipe/Reflect/ReflectionTraits.h"
#include "Pipe/Reflect/Type.h"
#include "Pipe/Reflect/TypeRegistry.h"


namespace p
{
	template<typename T>
	struct TCompiledTypeRegister
	{
		static inline Type* type = nullptr;

		TCompiledTypeRegister()
		{
			TypeRegistry::Get().RegisterCompiledType([] {
				InitType();
			});
		}


		static Type* InitType()
		{
			if constexpr (IsClass<T>() || IsStruct<T>())
			{
				type = T::InitType();
			}
			else if constexpr (IsReflectedEnum<T>())
			{
				type = reflection::TStaticEnumInitializer<T>::onInit();
			}
			else if constexpr (IsReflectedNative<T>())
			{
				type = reflection::TStaticNativeInitializer<T>::onInit();
			}
			return type;
		}

		static Type* GetType()
		{
			return type;
		}
	};
}    // namespace p
