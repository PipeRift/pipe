
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
		static constexpr sizet chunkSize = 1024;

		using Chunk = T*;

		template<typename Value>
		struct TPoolIterator final
		{
			using difference_type   = typename IdTraits<Id>::Difference;
			using value_type        = Value;
			using pointer           = Value*;
			using reference         = Value&;
			using iterator_category = std::random_access_iterator_tag;

			TPoolIterator() = default;

			TPoolIterator(Value* const* chunks, const difference_type index)
			    : chunks{chunks}, index{index}
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
				return (*chunks)[GetChunk(pos)][GetOffset(pos)];
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
				return std::addressof(chunks[GetChunk(pos)][GetOffset(pos)]);
			}

			reference operator*() const
			{
				const sizet pos = sizet(index - 1);
				return chunks[GetChunk(pos)][GetOffset(pos)];
			}

		private:
			Value* const* chunks = nullptr;
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
		TArray<Chunk> chunks;
		sizet size = 0;


	public:
		TPoolData() = default;

		T* Get(sizet index) const
		{
			return chunks[GetChunk(index)] + GetOffset(index);
		}

		void Reserve(sizet capacity)
		{
			const i32 neededChunks = GetChunk(capacity - 1) + 1;
			if (neededChunks <= chunks.Size())    // There are enough buckets
			{
				return;
			}

			chunks.Reserve(neededChunks);
			while (chunks.Size() < neededChunks)
			{
				chunks.Add(allocator.Alloc(chunkSize));
			}
		}

		// Release chunks to an specific size. No destructors are called
		void Release(sizet newSize)
		{
			CheckMsg(newSize >= size, "Cant erelease memory below used size");
			const i32 usedChunks = i32(newSize / chunkSize);
			if (usedChunks >= chunks.Size())
			{
				return;
			}

			for (auto pos = usedChunks; pos < chunks.Size(); ++pos)
			{
				allocator.Free(chunks[pos], chunkSize);
			}
			chunks.RemoveLast(chunks.Size() - usedChunks);
		}

		template<typename SetType>
		void ShrinkToSize(sizet size, const SetType& set)
		{
			// Destroy components
			for (auto i = size; i < set.Size(); ++i)
			{
				std::destroy_at(Get(i));
			}

			const auto from = (size + chunkSize - 1u) / chunkSize;
			for (auto pos = from; pos < chunks.Size(); ++pos)
			{
				allocator.Free(chunks[pos], chunkSize);
			}
			chunks.Resize(from);
		}

		T* Push(sizet index, T&& value)
		{
			CheckMsg(index < (chunks.Size() * chunkSize), "Out of bounds index");
			++size;
			T* instance = chunks[GetChunk(index)] + GetOffset(index);
			// Construct
			new (instance) T(Move(value));
			return instance;
		}

		T* Push(sizet index, const T& value)
		{
			CheckMsg(index < (chunks.Size() * chunkSize), "Out of bounds index");
			++size;
			T* instance = chunks[GetChunk(index)] + GetOffset(index);
			// Construct
			new (instance) T(Move(value));
			return instance;
		}

		void PopSwap(sizet index, sizet last)
		{
			T& item     = chunks[GetChunk(index)][GetOffset(index)];
			T& lastItem = chunks[GetChunk(last)][GetOffset(last)];
			Swap(item, lastItem);
			std::destroy_at(std::addressof(lastItem));
			--size;
		}
		void Pop(sizet index)
		{
			T& item = chunks[GetChunk(index)][GetOffset(index)];
			std::destroy_at(std::addressof(item));
			--size;
		}

		static i32 GetChunk(sizet index)
		{
			return i32(index / chunkSize);
		}

		static sizet GetOffset(sizet index)
		{
			return index & (chunkSize - 1);
		}

		ConstIterator cbegin() const
		{
			return ConstIterator{chunks.Data(), IdTraits<Id>::Difference(size)};
		}

		ConstIterator begin() const
		{
			return cbegin();
		}

		Iterator begin()
		{
			return Iterator{chunks.Data(), IdTraits<Id>::Difference(size)};
		}
		Iterator end()
		{
			return Iterator{chunks.Data(), {}};
		}
		Iterator end() const
		{
			return cend();
		}
		Iterator cend() const
		{
			return Iterator{chunks.Data(), {}};
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
