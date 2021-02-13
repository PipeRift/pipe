// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Memory/Alloc.h"


namespace Rift
{
	namespace Memory
	{
		class CORE_API Arena
		{
			const char* name;

		public:
			Arena(const char* name) : name(name) {}

			const char* GetName() const
			{
				return name;
			}
		};

		class CORE_API GlobalArena : public Arena
		{
			using Arena::Arena;

			static GlobalArena& GetInstance()
			{
				static GlobalArena arena{"Global"};
				return arena;
			}
		};
	}


	template <class T>
	struct STLAllocator
	{
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;

		// STD types


		STLAllocator() noexcept = default;
		STLAllocator(const STLAllocator&) noexcept = default;
		template <class U>
		STLAllocator(const STLAllocator<U>&) noexcept
		{}

		pointer allocate(size_type count)
		{
			return static_cast<pointer>(Rift::Alloc(count * sizeof(T)));
		}
		pointer allocate(size_type count, const void*)
		{
			return allocate(count);
		}
		void deallocate(pointer p, size_type)
		{
			Rift::Free(p);
		}

		using propagate_on_container_copy_assignment = std::true_type;
		using propagate_on_container_move_assignment = std::true_type;
		using propagate_on_container_swap = std::true_type;
		using is_always_equal = std::true_type;

		template <class U, class... Args>
		void construct(U* p, Args&&... args)
		{
			::new (p) U(std::forward<Args>(args)...);
		}
		template <class U>
		void destroy(U* p) noexcept
		{
			p->~U();
		}

		size_type max_size() const noexcept
		{
			return (PTRDIFF_MAX / sizeof(T));
		}
	};
}	 // namespace Rift::Memory
