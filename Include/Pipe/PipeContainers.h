// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Function.h"
#include "Pipe/Core/Log.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/Search.h"
#include "Pipe/Core/Sorting.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/Arena.h"

#include <iterator>


namespace p
{
	constexpr i32 GrowCapacity(i32 capacity, i32 newSize)
	{
		const i32 goldenCapacity = i32(capacity * 1.6);
		return p::math::Max(goldenCapacity, newSize);
	}

	template<typename Type>
	struct IRange;

	template<typename Type>
	struct TRangeIterator
	{
		using iterator_concept  = std::contiguous_iterator_tag;
		using iterator_category = std::random_access_iterator_tag;
		using value_type        = Type;
		using difference_type   = i32;
		using pointer           = Type*;
		using reference         = Type&;

	private:
		Type* ptr = nullptr;
#if P_DEBUG
		const IRange<Type>* range = nullptr;
#endif

	public:
		constexpr TRangeIterator() = default;
		constexpr TRangeIterator(Type* ptr, const IRange<Type>* range) noexcept
		    : ptr(ptr)
#if P_DEBUG
		    , range(range)
#endif
		{}

		constexpr Type& operator*() const noexcept
		{
			Verify();
			return *ptr;
		}

		constexpr Type* operator->() const noexcept
		{
			Verify();
			return ptr;
		}

		constexpr TRangeIterator& operator++() noexcept
		{
			Verify();
			++ptr;
			return *this;
		}

		constexpr TRangeIterator operator++(int) noexcept
		{
			TRangeIterator tmp = *this;
			++*this;
			return tmp;
		}

		constexpr TRangeIterator& operator--() noexcept
		{
			Verify();
			--ptr;
			return *this;
		}

		constexpr TRangeIterator operator--(int) noexcept
		{
			TRangeIterator tmp = *this;
			--*this;
			return tmp;
		}

		constexpr TRangeIterator& operator+=(const i32 offset) noexcept
		{
			VerifyOffset(offset);
			ptr += offset;
			return *this;
		}

		constexpr TRangeIterator operator+(const i32 offset) const noexcept
		{
			TRangeIterator tmp = *this;
			tmp += offset;
			return tmp;
		}

		friend constexpr TRangeIterator operator+(const i32 offset, TRangeIterator next) noexcept
		{
			next += offset;
			return next;
		}

		constexpr TRangeIterator& operator-=(const i32 offset) noexcept
		{
			return *this += -offset;
		}

		constexpr TRangeIterator operator-(const i32 offset) const noexcept
		{
			TRangeIterator tmp = *this;
			tmp -= offset;
			return tmp;
		}

		constexpr i32 operator-(const TRangeIterator& rhs) const noexcept
		{
			VerifyRange(rhs);
			return ptr - rhs.ptr;
		}

		constexpr Type& operator[](const i32 offset) const noexcept
		{
			return ptr[offset];
		}

		constexpr bool operator==(const TRangeIterator& rhs) const noexcept
		{
			VerifyRange(rhs);
			return ptr == rhs.ptr;
		}

		constexpr auto operator<=>(const TRangeIterator& rhs) const noexcept
		{
			VerifyRange(rhs);
			return ptr <=> rhs.ptr;
		}


	private:
		constexpr void Verify()
		{
#if P_DEBUG
			CheckMsg(ptr, "Iterator is null");
			CheckMsg(range->Data() <= ptr && ptr < (range->Data() + range->Size()),
			    "Iterator is out of range");
#endif
		}

		constexpr void VerifyOffset(const i32 offset) const noexcept
		{
#if P_DEBUG
			CheckMsg(offset == 0 || ptr, "Can not seek value-initialized iterator");
			if (offset < 0)
			{
				CheckMsg(offset >= range->Data() - ptr, "Can not seek iterator before range begin");
			}
			else if (offset > 0)
			{
				CheckMsg(offset <= (range->Data() + range->Size()) - ptr,
				    "Can not seek iterator after range end");
			}
#else
			(void)offset;
#endif
		}

		constexpr void VerifyRange(const TRangeIterator& rhs) const noexcept
		{
#if P_DEBUG
			CheckMsg(range == rhs.range, "Iterators dont share the same range");
#else
			(void)rhs;
#endif
		}
	};

	template<typename Type>
	class TReverseRangeIterator : public TRangeIterator<Type>
	{
	public:
		using Super = TRangeIterator<Type>;
		using Super::Super;    // Get constructors from RangeIterator


		constexpr TReverseRangeIterator& operator++() noexcept
		{
			return Super::operator--();
		}

		constexpr TReverseRangeIterator operator++(int) noexcept
		{
			TReverseRangeIterator tmp = *this;
			++*this;
			return tmp;
		}

		constexpr TReverseRangeIterator& operator--() noexcept
		{
			return Super::operator++();
		}

		constexpr TReverseRangeIterator operator--(int) noexcept
		{
			TReverseRangeIterator tmp = *this;
			--*this;
			return tmp;
		}

		constexpr TReverseRangeIterator& operator+=(const i32 offset) noexcept
		{
			return Super::operator-=(offset);
		}

		constexpr TReverseRangeIterator operator+(const i32 offset) const noexcept
		{
			return Super::operator-(offset);
		}

		friend constexpr TReverseRangeIterator operator+(
		    const i32 offset, TReverseRangeIterator next) noexcept
		{
			return Super::operator-(offset, next);
		}

		constexpr TReverseRangeIterator& operator-=(const i32 offset) noexcept
		{
			return Super::operator+=(offset);
		}

		constexpr TReverseRangeIterator operator-(const i32 offset) const noexcept
		{
			return Super::operator+(offset);
		}

		constexpr i32 operator-(const TReverseRangeIterator& rhs) const noexcept
		{
			return Super::operator+(rhs);
		}
	};


	/** Containers inheriting IRange point to a contiguous list of items of a type.
	 * NOTE: Memory might or might not be owned depending on the container itself.
	 */
	template<typename Type>
	struct IRange
	{
		using Iterator             = typename TRangeIterator<Type>;
		using ConstIterator        = typename TRangeIterator<const Type>;
		using ReverseIterator      = typename TReverseRangeIterator<Type>;
		using ConstReverseIterator = typename TReverseRangeIterator<const Type>;

	protected:
		Type* data = nullptr;
		i32 size   = 0;


		// Do not use IRange as a container itself. See TView and TArray2.
		IRange() = default;

	public:
		void Swap(i32 firstIndex, i32 secondIndex);

		i32 Size() const
		{
			return size;
		}

		i32 GetMemorySize() const
		{
			return Size() * sizeof(Type);
		}

		bool IsEmpty() const
		{
			return Size() == 0;
		}
		Type* Data() const
		{
			return data;
		}

		/// @returns a pointer to an element given a valid index, otherwise null
		Type* At(i32 index) const
		{
			return IsValidIndex(index) ? Data() + index : nullptr;
		}

		/**
		 * Array bracket operator. Returns reference to element at give index.
		 * @returns reference to indexed element.
		 */
		Type& operator[](i32 index) const
		{
			Check(IsValidIndex(index));
			return Data()[index];
		}

		Type& First() const
		{
			Check(size != 0);
			return Data()[0];
		}
		Type& Last() const
		{
			Check(size != 0);
			return Data()[size - 1];
		}

		bool IsValidIndex(i32 index) const
		{
			return index >= 0 && index < Size();
		}
		bool IsValidIndex(u32 index) const
		{
			return index < u32(Size());
		}


#pragma region Sort
		template<typename Predicate>
		void Sort(Predicate predicate)
		{
			p::Sort(Data(), Size(), predicate);
		}

		void Sort()
		{
			Sort(TLess<Type>());
		}

		template<typename Predicate>
		void SortRange(i32 firstIndex, i32 count, Predicate predicate)
		{
			const i32 maxSize = Size() - firstIndex;
			p::Sort(Data() + firstIndex, math::Min(count, maxSize), TLess<Type>());
		}

		void SortRange(i32 firstIndex, i32 count)
		{
			SortRange(firstIndex, count, TLess<Type>());
		}
#pragma endregion Sort

#pragma region Search
		Iterator FindIt(const Type& item) const
		{
			return std::find(begin(), end(), item);
		}

		Iterator FindIt(TFunction<bool(const Type&)> cb) const
		{
			return std::find_if(begin(), end(), cb);
		}

		i32 FindIndex(const Type& item) const
		{
			ConstIterator found = FindIt(item);
			if (found != end())
			{
				return i32(std::distance(begin(), found));
			}
			return NO_INDEX;
		}

		i32 FindIndex(TFunction<bool(const Type&)> cb) const
		{
			ConstIterator it = FindIt(Move(cb));
			if (it != end())
			{
				return i32(std::distance(begin(), it));
			}
			return NO_INDEX;
		}

		Type* Find(const Type& item) const
		{
			Iterator it = FindIt(item);
			return it != end() ? &*it : nullptr;
		}

		Type* Find(TFunction<bool(const Type&)> cb) const
		{
			Iterator it = FindIt(Move(cb));
			return it != end() ? &*it : nullptr;
		}

		Type& FindRef(const Type& item) const
		{
			Iterator it = FindIt(item);
			Check(it != end());
			return *it;
		}

		Type& FindRef(TFunction<bool(const Type&)> cb) const
		{
			Iterator it = FindIt(Move(cb));
			Check(it != end());
			return *it;
		}

		/**
		 * Finds the index of the first element that has a value greater than or equivalent to a
		 * specified value.
		 *
		 * @param sortPredicate used to sort the array. Default: a < b
		 * @return the furthermost iterator i in the range [first, last) such that
		 * for any iterator j in the range [first, i) the following corresponding
		 * condition holds: sortPredicate(*j, value) == false.
		 *
		 * Complexity: worst O(log(n)) | best O(1)
		 */
		template<typename Value, typename SortPredicate = TLess<>>
		i32 LowerBound(const Value& value, SortPredicate sortPredicate = {}) const
		{
			return p::LowerBound(Data(), 0, Size(), value, sortPredicate);
		}

		/**
		 * Finds the index of the first element that has a value that is greater than a specified
		 * value.
		 *
		 * @param sortPredicate used to sort the array. Default: a < b
		 * @return the furthermost iterator i in the range [first, last) such that
		 * for any iterator j in the range [first, i) the following corresponding
		 * condition holds: sortPredicate(value, *j) == false.
		 *
		 * Complexity: worst O(log(n)) | best O(1)
		 */
		template<typename Value, typename SortPredicate = TLess<>>
		i32 UpperBound(const Value& value, SortPredicate sortPredicate = {}) const
		{
			return p::UpperBound(Data(), 0, Size(), value, sortPredicate);
		}

		template<typename Value, typename SortPredicate = TLess<>>
		i32 FindSortedEqual(const Value& value, SortPredicate sortPredicate = {}) const
		{
			return p::BinarySearch(Data(), 0, Size(), value, sortPredicate);
		}

		template<typename Value>
		i32 FindSortedMax(const Value& max, bool included = false) const
		{
			return p::FindSortedMax(Data(), 0, Size(), max, included);
		}

		template<typename Value>
		i32 FindSortedMin(const Value& min, bool included = false) const
		{
			return p::FindSortedMin(Data(), 0, Size(), min, included);
		}

		bool Contains(const Type& item) const
		{
			return FindIt(item) != end();
		}

		bool Contains(TFunction<bool(const Type&)> cb) const
		{
			return FindIt(Move(cb)) != end();
		}

		template<typename Value, typename SortPredicate = TLess<>>
		bool ContainsSorted(const Value& value, SortPredicate sortPredicate = {}) const
		{
			return FindSortedEqual(value, sortPredicate) != NO_INDEX;
		}
#pragma endregion Search

		template<typename OtherType>
		bool operator==(const IRange<OtherType>& other) const
		{
			if (Size() != other.Size() || IsEmpty())
			{
				return false;
			}

			for (i32 i = 0; i < Size(); ++i)
			{
				if (Data()[i] != other[i])
				{
					return false;
				}
			}
			return true;
		}


		constexpr Iterator begin()
		{
			return Iterator(Data(), this);
		};
		constexpr ConstIterator begin() const
		{
			return ConstIterator(Data(), this);
		};
		constexpr Iterator end()
		{
			return Iterator(Data() + Size(), this);
		};
		constexpr ConstIterator end() const
		{
			return ConstIterator(Data() + Size(), this);
		};

		constexpr ReverseIterator rbegin()
		{
			return ReverseIterator(end());
		};
		constexpr ConstReverseIterator rbegin() const
		{
			return ConstReverseIterator(end());
		};
		constexpr ReverseIterator rend()
		{
			return ReverseIterator(begin());
		};
		constexpr ConstReverseIterator rend() const
		{
			return ConstReverseIterator(begin());
		};

		constexpr ConstIterator cbegin() const
		{
			return begin();
		};
		constexpr ConstIterator cend() const
		{
			return end();
		};
		constexpr ConstReverseIterator crbegin() const
		{
			return rbegin();
		};
		constexpr ConstReverseIterator crend() const
		{
			return rend();
		};
	};

	template<typename Type>
	struct TArray2 : public IRange<Type>
	{
	public:
		template<typename OtherType>
		friend struct TArray2;

	private:
		i32 capacity = 0;
		Arena& arena = p::GetCurrentArena();

	public:
		constexpr TArray2(Arena& arena = p::GetCurrentArena()) : arena{arena} {}
		constexpr TArray2(i32 count, Arena& arena = p::GetCurrentArena()) : arena{arena}
		{
			AddN(count);
		}
		constexpr TArray2(i32 count, const Type& value, Arena& arena = p::GetCurrentArena())
		    : arena{arena}
		{
			AddN(count, value);
		}
		TArray2(std::initializer_list<Type> initList) {}


#pragma region Insertion
		/*i32 Add(Type&& item)
		{
		    vector.push_back(Move(item));
		    return Size() - 1;
		}

		i32 Add(const Type& item)
		{
		    vector.push_back(item);
		    return Size() - 1;
		}*/

		constexpr i32 AddN(i32 count)
		{
			const i32 firstIndex = size;
			if (Ensure(count > 0))
			{
				GrowToInsert(count);
				size += count;
				ConstructItems(data + firstIndex, count);
			}
			return firstIndex;
		}

		constexpr i32 AddN(i32 count, const Type& item)
		{
			const i32 firstIndex = size;
			if (Ensure(count > 0))
			{
				GrowToInsert(count);
				size += count;
				ConstructItems(data + firstIndex, count, item);
			}
			return firstIndex;
		}

		/*Type& AddRef(Type&& item)
		{
		    return Data()[Add(Move(item))];
		}
		Type& AddRef(const Type& item)
		{
		    return Data()[Add(item)];
		}*/

		Type& AddNRef(i32 count)
		{
			return Data()[AddN(count)];
		}
		Type& AddNRef(i32 count, const Type& item)
		{
			return Data()[AddN(count, item)];
		}
#pragma endregion Insertion`


		/// Grows or shrinks the array´s reserved memory to fit N items, but never under the number
		/// it already contains.
		constexpr void Reserve(i32 newCapacity)
		{
			if (capacity < newCapacity)
			{
				Reallocate(newCapacity);
			}
		}

		constexpr void ReserveMore(i32 extraCapacity)
		{
			Reserve(size + extraCapacity);
		}

		/// Shrinks the array's reserved memory to fit just enough for the elements it contains.
		constexpr void Shrink()
		{
			if (size != capacity)
			{
				Reallocate(size);
			}
		}

		constexpr void Resize(i32 sizeNum) {}                       // TODO
		constexpr void Resize(i32 sizeNum, const Type& value) {}    // TODO

		/** Empty the array.
		 * @param shouldShrink false will not free memory
		 * @param newCapacity to how many elements the array should
		 */
		constexpr void Clear(bool shouldShrink = true, u32 newCapacity = 0)
		{
			DestroyItems(data, size);
			size = 0;
			if (shouldShrink)
			{
				Reserve(newCapacity);
			}
		}


	protected:
		constexpr void GrowToInsert(i32 extraSize)
		{
			const i32 newSize = size + extraSize;
			if (capacity < newSize)
			{
				Reallocate(GrowCapacity(capacity, newSize));
			}
		}

		constexpr void Reallocate(i32 newCapacity)
		{
			Check(capacity != newCapacity);

			if (size > newCapacity) [[unlikely]]
			{
				OnInvalidNum(newCapacity);
				return;
			}

			if (newCapacity > 0)
			{
				Type* newData = p::Alloc<Type>(arena, newCapacity);
				if (capacity > 0)    // There is memory to free and to possibly move/copy
				{
					if constexpr (IsMoveConstructible<Type> || !IsCopyConstructible<Type>)
					{
						MoveConstructItems<Type, true>(newData, size, data);
					}
					else
					{
						CopyConstructItems<Type, true>(newData, size, data);
					}
					Free(arena, data, capacity);
				}
				data     = newData;
				capacity = newCapacity;
			}
			else    // Nothing to reserve, just free all memory
			{
				Free(arena, data, capacity);
				data     = nullptr;
				capacity = 0;
			}
		}

		void OnInvalidNum(i32 newSize)
		{
			Error("Trying to resize TArray2 to an invalid size of {}", newSize);
		}
	};

	/** An array type with a fixed amount of elements. */
	template<typename Type, i32 Capacity>
	struct TFixedArray : public IRange<Type>
	{
	public:
		template<typename OtherType>
		friend struct TArray2;

		static constexpr i32 capacity = Capacity;

		Type inlineBuffer[capacity];
	};
};    // namespace p
