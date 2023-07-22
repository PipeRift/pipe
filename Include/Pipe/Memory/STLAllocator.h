// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Limits.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/Arena.h"

#include <type_traits>


namespace p
{
	template<typename T>
	struct STLAllocator
	{
		// STD types
		using value_type      = T;
		using size_type       = sizet;
		using reference       = value_type&;
		using const_reference = const value_type&;
		using pointer         = value_type*;
		using const_pointer   = const value_type*;
		template<typename U>
		struct rebind
		{
			using other = STLAllocator<U>;
		};

		Arena* arena = nullptr;


		STLAllocator(Arena& arena = GetCurrentArena()) : arena{&arena} {}
		STLAllocator(const STLAllocator& other) noexcept : arena{other.arena} {}
		template<typename U>
		STLAllocator(const STLAllocator<U>& other) noexcept : arena{other.arena}
		{}

		P_NODISCARD constexpr pointer allocate(size_type size)
		{
			return static_cast<pointer>(p::Alloc<T>(*arena, size));
		}
		constexpr pointer allocate(size_type size, const void*)
		{
			return allocate(size);
		}
		constexpr void deallocate(pointer p, size_type n)
		{
			p::Free<T>(*arena, p, n);
		}

		using propagate_on_container_copy_assignment = std::true_type;
		using propagate_on_container_move_assignment = std::true_type;
		using propagate_on_container_swap            = std::true_type;
		using is_always_equal                        = std::true_type;

		template<typename U, typename... Args>
		void construct(U* p, Args&&... args)
		{
			::new (static_cast<void*>(p)) U(Forward<Args>(args)...);
		}
		template<typename U>
		void destroy(U* p) noexcept
		{
			p->~U();
		}

		static constexpr size_type max_size() noexcept
		{
			return Limits<size_type>::Max() / sizeof(T);
		}
	};

	template<typename T1, typename T2>
	bool operator==(const STLAllocator<T1>& a, const STLAllocator<T2>& b) noexcept
	{
		return &a.arena == &b.arena;
	}
	template<typename T1, typename T2>
	bool operator!=(const STLAllocator<T1>& a, const STLAllocator<T2>& b) noexcept
	{
		return &a.arena != &b.arena;
	}
}    // namespace p
