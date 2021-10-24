// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Memory/Alloc.h"
#include "Memory/Allocators/DefaultAllocator.h"
#include "TypeTraits.h"


namespace Rift
{
	template<typename T, typename Allocator = Memory::DefaultAllocator>
	struct STLAllocator
	{
		// STD types
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
			typedef STLAllocator<U, Allocator> other;
		};

		Allocator allocator{};


		STLAllocator()                             = default;
		STLAllocator(const STLAllocator&) noexcept = default;
		template<class U>
		STLAllocator(const STLAllocator<U>&) noexcept
		{}

		pointer allocate(size_type count)
		{
			return static_cast<pointer>(allocator.Allocate(count * sizeof(T)));
		}
		pointer allocate(size_type count, const void*)
		{
			return allocate(count);
		}
		void deallocate(pointer p, size_type)
		{
			allocator.Free(p);
		}

		using propagate_on_container_copy_assignment = std::true_type;
		using propagate_on_container_move_assignment = std::true_type;
		using propagate_on_container_swap            = std::true_type;
		using is_always_equal                        = std::true_type;

		template<class U, class... Args>
		void construct(U* p, Args&&... args)
		{
			::new (p) U(std::forward<Args>(args)...);
		}
		template<class U>
		void destroy(U* p) noexcept
		{
			p->~U();
		}

		size_type max_size() const noexcept
		{
			return (PTRDIFF_MAX / sizeof(T));
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
}    // namespace Rift
