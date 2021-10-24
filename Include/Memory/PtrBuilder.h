// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Object/BaseObject.h"
#include "Reflection/ReflectionTraits.h"
#include "TypeTraits.h"


namespace Rift
{
	template<typename T>
	struct TOwnPtr;

	template<typename T>
	struct TPtrBuilder
	{
		template<typename... Args>
		static T* New(Args&&... args)
		{
			return new T(std::forward<Args>(args)...);
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
		static const bool value = std::is_void<decltype(Impl<T>(0))>::value;
	};
}    // namespace Rift
