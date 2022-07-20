
// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Core/Broadcast.h"
#include "Pipe/ECS/Id.h"
#include "Pipe/Memory/STLAllocator.h"

#include <memory>


namespace p::ecs
{
	template<typename T, typename Allocator>
	struct TPoolData
	{
		static constexpr sizet pageSize = 1024;

		using Page = T*;

		template<typename Value>
		struct TPoolIterator final
		{
			using difference_type   = typename IdTraits<Id>::Difference;
			using value_type        = Value;
			using pointer           = Value*;
			using reference         = Value&;
			using iterator_category = std::random_access_iterator_tag;

			TPoolIterator() = default;

			TPoolIterator(Value* const* pages, const difference_type index)
			    : pages{pages}, index{index}
			{}

			TPoolIterator& operator++()
			{
				return --index, *this;
			}

			TPoolIterator operator++(int)
			{
				TPoolIterator orig = *this;
				return ++(*this), orig;
			}

			TPoolIterator& operator--()
			{
				return ++index, *this;
			}

			TPoolIterator operator--(int)
			{
				TPoolIterator orig = *this;
				return operator--(), orig;
			}

			TPoolIterator& operator+=(const difference_type value)
			{
				index -= value;
				return *this;
			}

			TPoolIterator operator+(const difference_type value) const
			{
				TPoolIterator copy = *this;
				return (copy += value);
			}

			TPoolIterator& operator-=(const difference_type value)
			{
				return (*this += -value);
			}

			TPoolIterator operator-(const difference_type value) const
			{
				return (*this + -value);
			}

			difference_type operator-(const TPoolIterator& other) const
			{
				return other.index - index;
			}

			reference operator[](const difference_type value) const
			{
				const sizet pos{index - value - 1};
				return (*pages)[GetPage(pos)][GetOffset(pos)];
			}

			bool operator==(const TPoolIterator& other) const
			{
				return other.index == index;
			}

			bool operator!=(const TPoolIterator& other) const
			{
				return !(*this == other);
			}

			auto operator<=>(const TPoolIterator& other) const
			{
				return other.index <=> index;
			}

			pointer operator->() const
			{
				const sizet pos = sizet(index - 1);
				return pages[GetPage(pos)] + GetOffset(pos);
			}

			reference operator*() const
			{
				const sizet pos = sizet(index - 1);
				return pages[GetPage(pos)][GetOffset(pos)];
			}

		private:
			Value* const* pages = nullptr;
			difference_type index;
		};

		using Iterator = TPoolIterator<T>;
		/*! @brief Constant random access iterator type. */
		using ConstIterator = TPoolIterator<const T>;
		/*! @brief Reverse iterator type. */
		using ReverseIterator = std::reverse_iterator<Iterator>;
		/*! @brief Constant reverse iterator type. */
		using ConstReverseIterator = std::reverse_iterator<ConstIterator>;


	private:

		typename Allocator::template Typed<T> allocator;
		TArray<Page> pages;
		sizet size = 0;


	public:
		TPoolData() = default;

		T* Get(sizet index) const
		{
			return pages[GetPage(index)] + GetOffset(index);
		}

		void Reserve(sizet capacity)
		{
			const i32 neededPages = GetPage(capacity - 1) + 1;
			if (neededPages <= pages.Size())    // There are enough buckets
			{
				return;
			}

			pages.Reserve(neededPages);
			while (pages.Size() < neededPages)
			{
				pages.Add(allocator.Alloc(pageSize));
			}
		}

		// Release pages to an specific size. No destructors are called
		void Release(sizet newSize)
		{
			CheckMsg(newSize >= size, "Cant erelease memory below used size");
			const i32 usedPages = i32(newSize / pageSize);
			if (usedPages >= pages.Size())
			{
				return;
			}

			for (auto pos = usedPages; pos < pages.Size(); ++pos)
			{
				allocator.Free(pages[pos], pageSize);
			}
			pages.RemoveLast(pages.Size() - usedPages);
		}

		template<typename SetType>
		void ShrinkToSize(sizet size, const SetType& set)
		{
			// Destroy components
			for (auto i = size; i < set.Size(); ++i)
			{
				if (set.GetPageId(i) != NoId)
				{
					std::destroy_at(Get(i));
				}
			}

			const auto from = (size + pageSize - 1u) / pageSize;
			for (auto pos = from; pos < pages.Size(); ++pos)
			{
				allocator.Free(pages[pos], pageSize);
			}
			pages.Resize(from);
		}

		T* Push(sizet index, T&& value)
		{
			CheckMsg(index < (pages.Size() * pageSize), "Out of bounds index");
			++size;
			T* instance = pages[GetPage(index)] + GetOffset(index);
			// Construct
			new (instance) T(Move(value));
			return instance;
		}

		T* Push(sizet index, const T& value)
		{
			CheckMsg(index < (pages.Size() * pageSize), "Out of bounds index");
			++size;
			T* instance = pages[GetPage(index)] + GetOffset(index);
			// Construct
			new (instance) T(Move(value));
			return instance;
		}

		void PopSwap(sizet index, sizet last)
		{
			T& item     = pages[GetPage(index)][GetOffset(index)];
			T& lastItem = pages[GetPage(last)][GetOffset(last)];
			Swap(item, lastItem);
			std::destroy_at(std::addressof(lastItem));
			--size;
		}
		void Pop(sizet index)
		{
			T& item = pages[GetPage(index)][GetOffset(index)];
			std::destroy_at(std::addressof(item));
			--size;
		}

		static i32 GetPage(sizet index)
		{
			return i32(index / pageSize);
		}

		static sizet GetOffset(sizet index)
		{
			return index & (pageSize - 1);
		}

		ConstIterator cbegin() const
		{
			return ConstIterator{pages.Data(), IdTraits<Id>::Difference(size)};
		}

		ConstIterator begin() const
		{
			return cbegin();
		}

		Iterator begin()
		{
			return Iterator{pages.Data(), IdTraits<Id>::Difference(size)};
		}
		Iterator end()
		{
			return Iterator{pages.Data(), {}};
		}
		Iterator end() const
		{
			return cend();
		}
		Iterator cend() const
		{
			return Iterator{pages.Data(), {}};
		}

		ConstReverseIterator crbegin() const
		{
			return std::make_reverse_iterator(cend());
		}

		ConstReverseIterator rbegin() const
		{
			return crbegin();
		}

		ReverseIterator rbegin()
		{
			return std::make_reverse_iterator(end());
		}

		ConstReverseIterator crend() const
		{
			return std::make_reverse_iterator(cbegin());
		}

		ConstReverseIterator rend() const
		{
			return crend();
		}

		ReverseIterator rend()
		{
			return std::make_reverse_iterator(begin());
		}
	};


	template<typename T, typename Allocator>
		requires(IsEmpty<T>)
	struct TPoolData<T, Allocator>
	{
	public:
		TPoolData() = default;
		T* Get(sizet index) const
		{
			return nullptr;
		}
		void Reserve(sizet size) {}
		void Release(sizet size) {}

		template<typename SetType>
		void ShrinkToSize(sizet size, const SetType& set)
		{}

		T* Push(sizet index, const T&)
		{
			return nullptr;
		}
		T* Push(sizet index, T&&)
		{
			return nullptr;
		}
		void PopSwap(sizet index, sizet last) {}
		void Pop(sizet index) {}
	};
}    // namespace p::ecs
