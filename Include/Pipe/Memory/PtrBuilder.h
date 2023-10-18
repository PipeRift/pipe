// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Memory/Alloc.h"


namespace p
{
	template<typename T>
	struct TOwnPtr;

	template<typename T>
	struct TPtrBuilder
	{
		template<typename... Args>
		static T* New(Arena& arena, Args&&... args)
		{
			return new (p::Alloc<T>(arena)) T(p::Forward<Args>(args)...);
		}
		static void Delete(Arena& arena, void* rawPtr)
		{
			T* const ptr = static_cast<T*>(rawPtr);
			ptr->~T();
			p::Free<T>(arena, ptr);
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
