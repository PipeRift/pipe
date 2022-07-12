// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/ArenaAllocator.h"

#include <limits>
#include <memory>


namespace p
{
	template<typename T, typename Allocator = ArenaAllocator>
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
			using other = STLAllocator<U, Allocator>;
		};

		typename Allocator::template Typed<T> allocator{};


		STLAllocator()                             = default;
		STLAllocator(const STLAllocator&) noexcept = default;
		template<typename U>
		STLAllocator(const STLAllocator<U, Allocator>&) noexcept
		{}

		constexpr pointer allocate(size_type size)
		{
			return static_cast<pointer>(allocator.Alloc(size));
		}
		constexpr pointer allocate(size_type size, const void*)
		{
			return allocate(size);
		}
		constexpr void deallocate(pointer p, size_type n)
		{
			allocator.Free(p, n);
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
			return std::numeric_limits<size_type>::max() / sizeof(T);
		}
	};


	// Single parameter template type for pretemplated allocator arguments
	// template<typename> typename AllocatorType
	template<typename T>
	using STLDefaultAllocator = STLAllocator<T>;

	template<typename T1, typename T2, typename Allocator>
	bool operator==(const STLAllocator<T1, Allocator>&, const STLAllocator<T2, Allocator>&) noexcept
	{
		return true;
	}
	template<typename T1, typename T2, typename Allocator>
	bool operator!=(const STLAllocator<T1, Allocator>&, const STLAllocator<T2, Allocator>&) noexcept
	{
		return false;
	}
}    // namespace p
