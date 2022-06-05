// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Reflection/BaseClass.h"
#include "Reflection/ReflectionTraits.h"
#include "TypeTraits.h"


namespace p
{
	template<typename T>
	struct TOwnPtr;

	template<typename T>
	struct TPtrBuilder
	{
		template<typename... Args>
		static T* New(Args&&... args)
		{
			return new T(Forward<Args>(args)...);
		}
		static void PostNew(TOwnPtr<T>& ptr) {}
		static void Delete(void* rawPtr)
		{
			delete static_cast<T*>(rawPtr);
		}


		static T* NewArray(sizet size)
		{
			using Elem = std::remove_extent_t<T>;
			return new Elem[size]();
		}
		static void PostNewArray(TOwnPtr<T>& ptr, sizet size) {}
		static void DeleteArray(void* rawPtr)
		{
			delete[] static_cast<T*>(rawPtr);
		}
	};

	template<typename T>
	struct HasCustomPtrBuilder
	{
	private:
		template<typename V>
		static void Impl(decltype(typename V::template PtrBuilder<T>(), int()));
		template<typename V>
		static bool Impl(char);

	public:
		static constexpr bool value = std::is_void<decltype(Impl<T>(0))>::value;
	};
}    // namespace p
