// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Limits.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/Arena.h"
#include "PipePlatform.h"

#include <type_traits>



namespace p
{
	template<typename T>
	struct STLAllocator
	{
		using value_type      = T;
		using size_type       = sizet;
		using difference_type = std::ptrdiff_t;
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


		STLAllocator(Arena& arena = GetCurrentArena()) noexcept : arena{&arena} {}
		STLAllocator(const STLAllocator& other) noexcept : arena{other.arena} {}
		template<typename U>
		STLAllocator(const STLAllocator<U>& other) noexcept : arena{other.arena}
		{}
		STLAllocator select_on_container_copy_construction() const
		{
			return *this;
		}

#if (__cplusplus >= 201703L)    // C++17
		P_NODISCARD T* allocate(size_type count)
		{
			return p::Alloc<T>(*arena, count);
		}
		P_NODISCARD T* allocate(size_type count, const void*)
		{
			return allocate(count);
		}
#else
		P_NODISCARD pointer allocate(size_type count, const void* = 0)
		{
			return p::Alloc<T>(*arena, count);
		}
#endif
		constexpr void deallocate(T* p, size_type n)
		{
			p::Free<T>(*arena, p, n);
		}


#if ((__cplusplus >= 201103L) || (_MSC_VER > 1900))    // C++11
		using is_always_equal                        = std::true_type;
		using propagate_on_container_copy_assignment = std::true_type;
		using propagate_on_container_move_assignment = std::true_type;
		using propagate_on_container_swap            = std::true_type;
		template<typename U, typename... Args>
		void construct(U* p, Args&&... args)
		{
			::new (p) U(p::Forward<Args>(args)...);
		}
		template<typename U>
		void destroy(U* p) noexcept
		{
			p->~U();
		}
#else
		void construct(pointer p, value_type const& val)
		{
			::new (p) value_type(val);
		}
		void destroy(pointer p)
		{
			p->~value_type();
		}
#endif

		size_type max_size() const noexcept
		{
			return (Limits<difference_type>::Max() / sizeof(value_type));
		}
		pointer address(reference x) const
		{
			return &x;
		}
		const_pointer address(const_reference x) const
		{
			return &x;
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
