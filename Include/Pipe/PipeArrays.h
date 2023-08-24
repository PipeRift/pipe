// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Function.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/Search.h"
#include "Pipe/Core/Sorting.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/Arena.h"
#include "Pipe/Memory/Memory.h"
#include "Pipe/PipeArraysFwd.h"

#include <initializer_list>
#include <iterator>


namespace p
{
	////////////////////////////////
	// FORWARD DECLARATIONS
	//

	template<typename Type>
	struct IArray;

	template<typename Type, u32 InlineCapacity>
	struct TInlineArray;


	////////////////////////////////
	// RANGES
	//

	template<typename Type>
	struct TArrayIterator
	{
		using iterator_concept  = std::contiguous_iterator_tag;
		using iterator_category = std::random_access_iterator_tag;
		using value_type        = Type;
		using difference_type   = i32;
		using pointer           = Type*;
		using reference         = Type&;

		using ContainerType = IArray<Type>;

		template<typename OtherType>
		friend struct TArrayIterator;


	private:
		Type* ptr = nullptr;
#if P_DEBUG
		const ContainerType* range = nullptr;
#endif

	public:
		constexpr TArrayIterator() = default;
		constexpr TArrayIterator(Type* ptr, const ContainerType* range) noexcept
		    : ptr(ptr)
#if P_DEBUG
		    , range(range)
#endif
		{}

		constexpr TArrayIterator(const TArrayIterator& other)
		    : ptr(other.ptr)
#if P_DEBUG
		    , range(other.range)
#endif
		{}
		TArrayIterator& operator=(const TArrayIterator& other)
		{
			ptr = other.ptr;
#if P_DEBUG
			range = other.range;
#endif
			return *this;
		}

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

		constexpr TArrayIterator& operator++() noexcept
		{
			Verify();
			++ptr;
			return *this;
		}

		constexpr TArrayIterator operator++(int) noexcept
		{
			TArrayIterator tmp = *this;
			++*this;
			return tmp;
		}

		constexpr TArrayIterator& operator--() noexcept
		{
			Verify();
			--ptr;
			return *this;
		}

		constexpr TArrayIterator operator--(int) noexcept
		{
			TArrayIterator tmp = *this;
			--*this;
			return tmp;
		}

		constexpr TArrayIterator& operator+=(const i32 offset) noexcept
		{
			VerifyOffset(offset);
			ptr += offset;
			return *this;
		}

		constexpr TArrayIterator operator+(const i32 offset) const noexcept
		{
			TArrayIterator tmp = *this;
			tmp += offset;
			return tmp;
		}

		friend constexpr TArrayIterator operator+(const i32 offset, TArrayIterator next) noexcept
		{
			next += offset;
			return next;
		}

		constexpr TArrayIterator& operator-=(const i32 offset) noexcept
		{
			return *this += -offset;
		}

		constexpr TArrayIterator operator-(const i32 offset) const noexcept
		{
			TArrayIterator tmp = *this;
			tmp -= offset;
			return tmp;
		}

		constexpr i32 operator-(const TArrayIterator& rhs) const noexcept
		{
			VerifyRange(rhs);
			return ptr - rhs.ptr;
		}

		constexpr Type& operator[](const i32 offset) const noexcept
		{
			return ptr[offset];
		}

		constexpr bool operator==(const TArrayIterator& rhs) const noexcept
		{
			VerifyRange(rhs);
			return ptr == rhs.ptr;
		}

		constexpr auto operator<=>(const TArrayIterator& rhs) const noexcept
		{
			VerifyRange(rhs);
			return ptr <=> rhs.ptr;
		}

		constexpr operator TArrayIterator<const Type>&() requires IsMutable<Type>
		{
			return *reinterpret_cast<TArrayIterator<const Type>*>(this);
		}
		constexpr operator const TArrayIterator<const Type>&() const requires IsMutable<Type>
		{
			return *reinterpret_cast<TArrayIterator<const Type>*>(this);
		}


	private:
		constexpr void Verify() const
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

		constexpr void VerifyRange(const TArrayIterator& rhs) const noexcept
		{
#if P_DEBUG
			CheckMsg(range == rhs.range, "Iterators dont share the same range");
#else
			(void)rhs;
#endif
		}
	};

	template<typename Type>
	class TReverseArrayIterator : public TArrayIterator<Type>
	{
	public:
		using Super = TArrayIterator<Type>;
		using Super::Super;    // Get constructors from ArrayIterator


		constexpr TReverseArrayIterator& operator++() noexcept
		{
			return Super::operator--();
		}

		constexpr TReverseArrayIterator operator++(int) noexcept
		{
			TReverseArrayIterator tmp = *this;
			++*this;
			return tmp;
		}

		constexpr TReverseArrayIterator& operator--() noexcept
		{
			return Super::operator++();
		}

		constexpr TReverseArrayIterator operator--(int) noexcept
		{
			TReverseArrayIterator tmp = *this;
			--*this;
			return tmp;
		}

		constexpr TReverseArrayIterator& operator+=(const i32 offset) noexcept
		{
			return Super::operator-=(offset);
		}

		constexpr TReverseArrayIterator operator+(const i32 offset) const noexcept
		{
			return Super::operator-(offset);
		}

		friend constexpr TReverseArrayIterator operator+(
		    const i32 offset, TReverseArrayIterator next) noexcept
		{
			return Super::operator-(offset, next);
		}

		constexpr TReverseArrayIterator& operator-=(const i32 offset) noexcept
		{
			return Super::operator+=(offset);
		}

		constexpr TReverseArrayIterator operator-(const i32 offset) const noexcept
		{
			return Super::operator+(offset);
		}

		constexpr i32 operator-(const TReverseArrayIterator& rhs) const noexcept
		{
			return Super::operator+(rhs);
		}
	};


	/** Containers inheriting IArray point to a contiguous list of elements of a type.
	 * NOTE: Memory might or might not be owned depending on the container itself.
	 */
	template<typename Type>
	struct IArray
	{
		using Iterator             = typename TArrayIterator<Type>;
		using ConstIterator        = typename TArrayIterator<const Type>;
		using ReverseIterator      = typename TReverseArrayIterator<Type>;
		using ConstReverseIterator = typename TReverseArrayIterator<const Type>;

		using ItemType = Type;

	protected:
		Type* data = nullptr;
		i32 size   = 0;


		// Do not use IArray as a container itself. See TView, TArray and TInlineArray.
		constexpr IArray() = default;

	public:
		constexpr Type* Data() const
		{
			return data;
		}

		constexpr i32 Size() const
		{
			return size;
		}

		constexpr bool IsEmpty() const
		{
			return size == 0;
		}

		constexpr bool IsValidIndex(i32 index) const
		{
			return index >= 0 && index < size;
		}
		constexpr bool IsValidIndex(u32 index) const
		{
			return index < u32(size);
		}
		constexpr bool IsValidIndexRange(i32 index, i32 count) const
		{
			return index >= 0 && (index + count) <= size;
		}

		constexpr i32 GetMemorySize() const
		{
			return size * sizeof(Type);
		}

		/// @returns a pointer to an element given a valid index, otherwise null
		constexpr Type* At(i32 index) const
		{
			return IsValidIndex(index) ? data + index : nullptr;
		}

		/**
		 * Array bracket operator. Returns reference to element at give index.
		 * @returns reference to indexed element.
		 */
		constexpr Type& operator[](i32 index) const
		{
			Check(IsValidIndex(index));
			return data[index];
		}

		constexpr operator IArray<const Type>&() requires(IsMutable<Type>)
		{
			return *reinterpret_cast<IArray<const Type>*>(this);
		}
		constexpr operator const IArray<const Type>&() const requires(IsMutable<Type>)
		{
			return *reinterpret_cast<const IArray<const Type>*>(this);
		}

		constexpr Type& First() const
		{
			Check(size != 0);
			return data[0];
		}
		constexpr Type& Last() const
		{
			Check(size != 0);
			return data[size - 1];
		}


#pragma region Sort
		void Swap(i32 firstIdx, i32 secondIdx)
		{
			if (IsValidIndex(firstIdx) && IsValidIndex(secondIdx) && firstIdx != secondIdx)
			{
				SwapUnsafe(firstIdx, secondIdx);
			}
		}

		void Swap(i32 firstIdx, i32 secondIdx, i32 count)
		{
			// Check that ranges fit inside the array and don't overlap
			if (IsValidIndexRange(firstIdx, count) && IsValidIndexRange(secondIdx, count)
			    && (firstIdx >= (secondIdx + count) || secondIdx >= (firstIdx + count)))
			{
				SwapUnsafe(firstIdx, secondIdx, count);
			}
		}

		void SwapUnsafe(i32 firstIdx, i32 secondIdx)
		{
			p::Swap(data[firstIdx], data[secondIdx]);
		}

		void SwapUnsafe(i32 firstIdx, i32 secondIdx, i32 count)
		{
			for (i32 i = 0; i < count; ++i)
			{
				p::Swap(data[firstIdx + i], data[secondIdx + i]);
			}
		}

		template<typename Predicate>
		void Sort(Predicate predicate)
		{
			p::Sort(data, size, predicate);
		}

		void Sort()
		{
			Sort(TLess<Type>());
		}

		template<typename Predicate>
		void SortRange(i32 firstIdx, i32 count, Predicate predicate)
		{
			const i32 maxSize = Size() - firstIdx;
			p::Sort(data + firstIdx, math::Min(count, maxSize), TLess<Type>());
		}

		void SortRange(i32 firstIdx, i32 count)
		{
			SortRange(firstIdx, count, TLess<Type>());
		}
#pragma endregion Sort


#pragma region Search
		Iterator FindIt(const Type& value) const
		{
			for (Type *p = data, *end = data + size; p != end; ++p)
			{
				if (*p == value)
				{
					return {p, this};
				}
			}
			return end();
		}

		Iterator FindIt(TFunction<bool(const Type&)> cb) const
		{
			for (Type *p = data, *end = data + size; p != end; ++p)
			{
				if (cb(*p))
				{
					return {p, this};
				}
			}
			return end();
		}

		i32 FindIndex(const Type& value) const
		{
			ConstIterator found = FindIt(value);
			if (found != end())
			{
				return i32(found - begin());
			}
			return NO_INDEX;
		}

		i32 FindIndex(TFunction<bool(const Type&)> cb) const
		{
			ConstIterator found = FindIt(Move(cb));
			if (found != end())
			{
				return i32(found - begin());
			}
			return NO_INDEX;
		}

		Type* Find(const Type& value) const
		{
			Iterator it = FindIt(value);
			return it != end() ? &*it : nullptr;
		}

		Type* Find(TFunction<bool(const Type&)> cb) const
		{
			Iterator it = FindIt(Move(cb));
			return it != end() ? &*it : nullptr;
		}

		Type& FindRef(const Type& value) const
		{
			Iterator it = FindIt(value);
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
		 * Finds the index of the first element that has a value that is greater than a
		 * specified value.
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

		bool Contains(const Type& value) const
		{
			return FindIt(value) != end();
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
		bool operator==(const IArray<OtherType>& other) const
		{
			if (size != other.size || IsEmpty())
			{
				return false;
			}

			for (i32 i = 0; i < size; ++i)
			{
				if (data[i] != other[i])
				{
					return false;
				}
			}
			return true;
		}


		template<typename Callback>
		void Each(Callback cb) const
		{
			for (i32 i = 0; i < size; ++i)
			{
				cb(data[i]);
			}
		}

		template<typename Callback>
		void EachReverse(Callback cb) const
		{
			for (i32 i = size - 1; i >= 0; --i)
			{
				cb(data[i]);
			}
		}

		constexpr Iterator begin() const
		{
			return Iterator(data, this);
		};
		constexpr Iterator end() const
		{
			return Iterator(data + size, this);
		};

		constexpr ReverseIterator rbegin() const
		{
			return ReverseIterator(end());
		};
		constexpr ReverseIterator rend() const
		{
			return ReverseIterator(begin());
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


	////////////////////////////////
	// ARRAYS
	//

	/** An array type with a fixed amount of elements. */
	template<typename Type, u32 InlineCapacity>
	struct TInlineArray : public IArray<Type>
	{
		using Super = IArray<Type>;

	public:
		template<typename OtherType, u32 OtherInlineCapacity>
		friend struct TInlineArray;

		static constexpr i32 inlineCapacity = InlineCapacity;

		i32 capacity = 0;
		Arena* arena = &p::GetCurrentArena();
		mutable TTypeAsBytesArray<Type, InlineCapacity>
		    inlineBuffer;    // Not used if InlineCapacity is 0


#pragma region Constructors
	public:
		constexpr TInlineArray(Arena& arena = p ::GetCurrentArena()) : arena{&arena} {}
		constexpr TInlineArray(i32 initialSize, Arena& arena = p::GetCurrentArena()) : arena{&arena}
		{
			Assign(initialSize);
		}
		constexpr TInlineArray(
		    i32 initialSize, const Type& value, Arena& arena = p::GetCurrentArena())
		    : arena{&arena}
		{
			Assign(initialSize, value);
		}
		constexpr TInlineArray(std::initializer_list<Type> initList)
		{
			Assign(Move(initList));
		}
		template<typename It>
		constexpr TInlineArray(
		    const It& beginIt, const It& endIt, Arena& arena = p::GetCurrentArena())
		    : arena{&arena}
		{
			Append(beginIt, endIt);
		}
		TInlineArray(const Type* data, i32 sizeNum, Arena& arena = p::GetCurrentArena())
		    : arena{&arena}
		{
			Assign(data, sizeNum);
		}
		TInlineArray(const Type* first, const Type* last, Arena& arena = p::GetCurrentArena())
		    : arena{&arena}
		{
			Assign(first, std::distance(first, last));
		}
		template<u32 OtherInlineCapacity>
		TInlineArray(TInlineArray<Type, OtherInlineCapacity>&& other)
		{
			MoveFrom(Forward<TInlineArray<Type, OtherInlineCapacity>>(other));
		}
		template<u32 OtherInlineCapacity>
		TInlineArray<Type, InlineCapacity>& operator=(
		    TInlineArray<Type, OtherInlineCapacity>&& other)
		{
			if (this != (void*)&other)
			{
				MoveFrom(Forward<TInlineArray<Type, OtherInlineCapacity>>(other));
			}
			return *this;
		}
		TInlineArray(const TInlineArray& other)
		{
			CopyFrom(other);
		}
		TInlineArray& operator=(const TInlineArray& other)
		{
			if (this != (void*)&other)
			{
				CopyFrom(other);
			}
			return *this;
		}
		TInlineArray(const IArray<const Type>& other)
		{
			CopyFrom(other);
		}
		TInlineArray& operator=(const IArray<const Type>& other)
		{
			if (this != (void*)&other)
			{
				CopyFrom(other);
			}
			return *this;
		}
		~TInlineArray()
		{
			Clear(true);
		}
#pragma endregion Constructors


#pragma region Insertions
	public:
		i32 Add()
		{
			const i32 firstIdx = Super::size;
			AddUninitialized(1);
			new (Super::data + firstIdx) Type();
			return firstIdx;
		}
		i32 Add(Type&& value)
		{
			const i32 firstIdx = Super::size;
			AddUninitialized(1);
			new (Super::data + firstIdx) Type(Forward<Type>(value));
			return firstIdx;
		}
		i32 Add(const Type& value)
		{
			const i32 firstIdx = Super::size;
			AddUninitialized(1);
			new (Super::data + firstIdx) Type(value);
			return firstIdx;
		}
		Type& AddRef()
		{
			return Super::data[Add()];
		}
		Type& AddRef(Type&& value)
		{
			return Super::data[Add(Forward<Type>(value))];
		}
		Type& AddRef(const Type& value)
		{
			return Super::data[Add(value)];
		}

		template<typename SortPredicate = TLess<Type>>
		i32 AddSorted(Type&& item, SortPredicate sortPredicate = {})
		{
			const i32 index = Super::LowerBound(item, sortPredicate);
			if (index != NO_INDEX)
			{
				Insert(index, Move(item));
				return index;
			}
			return Add(Move(item));
		}

		template<typename SortPredicate = TLess<Type>>
		i32 AddSorted(const Type& item, SortPredicate sortPredicate = {})
		{
			const i32 index = Super::LowerBound(item, sortPredicate);
			if (index != NO_INDEX)
			{
				Insert(index, item);
				return index;
			}
			return Add(item);
		}

		i32 AddUnique(const Type& value)
		{
			const i32 found = Super::FindIndex(value);
			if (found == NO_INDEX)
			{
				return Add(value);
			}
			return found;
		}

		/** Finds or adds an element in a sorted array.
		 * Much more efficient that AddUnique (but must be sorted).
		 * NOTE: Undefined behavior on unsorted arrays!
		 * @param value to find or add
		 * @param sortPredicate used to sort the array
		 * @param insertSorted when true(default) inserts element sorted
		 * @return index, added
		 */
		template<typename SortPredicate = TLess<Type>>
		i32 AddUniqueSorted(const Type& value, SortPredicate sortPredicate = {},
		    bool* outFound = nullptr, bool insertSorted = true)
		{
			const i32 index = Super::LowerBound(value, sortPredicate);
			if (index != NO_INDEX)
			{
				if (!sortPredicate(value, Super::data[index]))    // Equal check, found element
				{
					if (outFound)
						*outFound = false;
					return index;
				}
				else if (insertSorted)
				{
					Insert(index, value);
					if (outFound)
						*outFound = true;
					return index;
				}
			}

			if (outFound)
				*outFound = true;
			return Add(value);
		}

		template<typename SortPredicate = TLess<Type>>
		i32 AddUniqueSorted(Type&& value, SortPredicate sortPredicate = {},
		    bool* outFound = nullptr, bool insertSorted = true)
		{
			const i32 index = Super::LowerBound(value, sortPredicate);
			if (index != NO_INDEX)
			{
				if (!sortPredicate(value, Super::data[index]))    // Equal check, found element
				{
					if (outFound)
						*outFound = false;
					return index;
				}
				else if (insertSorted)
				{
					Insert(index, Forward<Type>(value));
					if (outFound)
						*outFound = true;
					return index;
				}
			}

			if (outFound)
				*outFound = true;
			return Add(Forward<Type>(value));
		}

		void Append(i32 count)
		{
			const i32 firstIdx = Super::size;
			AddUninitialized(count);
			ConstructItems(Super::data + firstIdx, count);
		}
		void Append(i32 count, const Type& value)
		{
			const i32 firstIdx = Super::size;
			AddUninitialized(count);
			ConstructItems(Super::data + firstIdx, count, value);
		}
		void Append(const Type* values, i32 count)
		{
			const i32 firstIdx = Super::size;
			AddUninitialized(count);
			CopyConstructItems(Super::data + firstIdx, count, values);
		}
		void Append(const IArray<const Type>& values)
		{
			Append(values.Data(), values.Size());
		}
		void Append(std ::initializer_list<Type> initList)
		{
			const i32 count    = i32(initList.size());
			const i32 firstIdx = Super::size;
			AddUninitialized(count);
			MoveOrCopyConstructItems(Super::data + firstIdx, count, initList.begin());
		}
		template<typename It>
		void Append(const It& beginIt, const It& endIt)
		{
			ReserveMore(std::distance(beginIt, endIt));
			for (It it = beginIt; it != endIt; ++it)
			{
				Add(*it);
			}
		}

		void Assign(i32 count)
		{
			Clear(false);
			Reserve(count);
			Super::size = count;
			ConstructItems(Super::data, count);
		}
		void Assign(i32 count, const Type& value)
		{
			Clear(false);
			Reserve(count);
			Super::size = count;
			ConstructItems(Super::data, count, value);
		}
		void Assign(const Type* values, i32 count)
		{
			Clear(false);
			Reserve(count);
			Super::size = count;
			CopyConstructItems(Super::data, count, values);
		}
		void Assign(const IArray<Type>& values)
		{
			CopyFrom(values);
		}
		template<i32 OtherInlineCapacity>
		void Assign(TInlineArray<Type, OtherInlineCapacity>&& values) requires(IsMutable<Type>)
		{
			MoveFrom(Forward<TInlineArray<Type, OtherInlineCapacity>>(values));
		}
		void Assign(std ::initializer_list<Type> initList)
		{
			Assign(initList.begin(), i32(initList.size()));
		}

		void Insert(i32 atIndex)
		{
			InsertUninitialized(atIndex, 1);
			new (Super::data + atIndex) Type();
		}
		void Insert(i32 atIndex, Type&& value)
		{
			InsertUninitialized(atIndex, 1);
			new (Super::data + atIndex) Type(Forward<Type>(value));
		}
		void Insert(i32 atIndex, const Type& value)
		{
			InsertUninitialized(atIndex, 1);
			new (Super::data + atIndex) Type(value);
		}
		void Insert(i32 atIndex, i32 count, const Type& value)
		{
			InsertUninitialized(atIndex, count);
			ConstructItems(Super::data + atIndex, count, value);
		}
		void Insert(i32 atIndex, const Type* values, i32 count)
		{
			InsertUninitialized(atIndex, count);
			CopyConstructItems(Super::data + atIndex, count, values);
		}
		void Insert(i32 atIndex, const IArray<Type>& values)
		{
			Insert(atIndex, values.Data(), values.Size());
		}
#pragma endregion Insertions


#pragma region Removals
	public:
		/**
		 * Delete all items that match another provided item
		 * @return number of deleted items
		 */
		i32 Remove(const Type& item, bool shouldShrink = true)
		{
			return RemoveAt(Super::FindIndex(item), shouldShrink);
		}

		i32 Remove(const IArray<Mut<Type>>& items, bool shouldShrink = true)
		{
			const i32 lastSize = Super::size;
			for (i32 i = 0; i < Super::size; ++i)
			{
				if (items.Contains(Super::data[i]))
				{
					RemoveAtUnsafe(i, false);
					--i;    // Repeat same index
				}
			}
			if (shouldShrink)
			{
				Shrink();
			}
			return lastSize - Super::size;
		}

		i32 Remove(const IArray<const Type>& items, bool shouldShrink = true)
		{
			const i32 lastSize = Super::size;
			for (i32 i = 0; i < Super::size; ++i)
			{
				if (items.Contains(Super::data[i]))
				{
					RemoveAtUnsafe(i, false);
					--i;    // Repeat same index
				}
			}
			if (shouldShrink)
			{
				Shrink();
			}
			return lastSize - Super::size;
		}

		// Removes an item assuming the array is sorted
		template<typename OtherType, typename SortPredicate = TLess<>>
		bool RemoveSorted(const OtherType& value, SortPredicate sortPredicate = {},
		    const bool shouldShrink = true)
		{
			return RemoveAt(Super::FindSortedEqual(value, sortPredicate));
		}

		/**
		 * Delete item at index
		 * @return true if removed
		 */
		bool RemoveAt(i32 index, const bool shouldShrink = true)
		{
			if (Super::IsValidIndex(index))
			{
				RemoveAtUnsafe(index, shouldShrink);
				return true;
			}
			return false;
		}

		/**
		 * Delete N items at index
		 * @return true if removed
		 */
		bool RemoveAt(i32 index, i32 count, const bool shouldShrink = true)
		{
			if (Super::IsValidIndexRange(index, count))
			{
				RemoveAtUnsafe(index, count, shouldShrink);
				return true;
			}
			return false;
		}

		/**
		 * Delete item at an index by swapping with the last element. Doesn't preserve order.
		 * Faster than RemoveAt since it doesnt push all remaining elements 1 position forward.
		 * @return true if removed
		 */
		bool RemoveAtSwap(i32 index, const bool shouldShrink = true)
		{
			if (Super::IsValidIndex(index))
			{
				RemoveAtSwapUnsafe(index, shouldShrink);
				return true;
			}
			return false;
		}

		/**
		 * Delete N items at an index by swapping with the last elements. Doesn't preserve order.
		 * Faster than RemoveAt since it doesnt push all remaining elements N positions forward.
		 * @return true if removed
		 */
		bool RemoveAtSwap(i32 index, i32 count, const bool shouldShrink = true)
		{
			if (Super::IsValidIndexRange(index, count))
			{
				RemoveAtSwapUnsafe(index, count, shouldShrink);
				return true;
			}
			return false;
		}

		/**
		 * Delete item at index.
		 * Unsafe version. Doesn't make sure index is valid.
		 * @return true if removed
		 */
		void RemoveAtUnsafe(i32 index, const bool shouldShrink = true)
		{
			const i32 lastIndex   = index + 1;
			const i32 countToPull = Super::size - lastIndex;
			DestroyItems(Super::data + index, 1);
			MoveItems(Super::data + index, countToPull, Super::data + lastIndex);
			--Super::size;

			/// @OPTIMIZE: Shrinking can be combined to avoid moving trailing elements twice
			if (shouldShrink)
				Shrink();
		}

		/**
		 * Delete N items at index.
		 * Unsafe version. Doesn't make sure index is valid.
		 * @return true if removed
		 */
		void RemoveAtUnsafe(i32 index, i32 count, const bool shouldShrink = true)
		{
			const i32 lastIndex   = index + count;
			const i32 countToPull = Super::size - lastIndex;
			DestroyItems(Super::data + index, count);
			MoveItems(Super::data + index, countToPull, Super::data + lastIndex);
			Super::size -= count;

			/// @OPTIMIZE: Shrinking can be combined to avoid moving trailing elements twice
			if (shouldShrink)
				Shrink();
		}

		/**
		 * Delete item at an index by swapping with the last element. Doesn't preserve order.
		 * Faster than RemoveAt since it doesnt push all remaining elements 1 position left.
		 * Unsafe version. Doesn't make sure index is valid!
		 * @return true if removed
		 */
		void RemoveAtSwapUnsafe(i32 index, const bool shouldShrink = true)
		{
			const i32 lastIndex = Super::size - 1;
			Super::SwapUnsafe(index, lastIndex);
			RemoveAtUnsafe(lastIndex, shouldShrink);
		}

		/**
		 * Delete item at an index by swapping with the last element. Doesn't preserve order.
		 * Faster than RemoveAt since it doesnt push all remaining elements 1 position left.
		 * Unsafe version. Doesn't make sure index is valid!
		 * @return true if removed
		 */
		void RemoveAtSwapUnsafe(i32 index, i32 count, const bool shouldShrink = true)
		{
			const i32 lastIndex   = Super::size - count;
			const i32 countToSwap = math::Min(count, lastIndex - index);
			Super::SwapUnsafe(
			    index, Super::size - countToSwap, countToSwap);    // Dont swap more than those left
			RemoveAtUnsafe(lastIndex, count, shouldShrink);
		}

		/**
		 * Delete all items that match a delegate
		 * @return number of deleted items
		 */
		i32 RemoveIf(TFunction<bool(const Type&)>&& callback, const bool shouldShrink = true)
		{
			const i32 lastSize = Super::size;
			// We remove from the back so that there are less elements to move when removing an
			// element
			for (i32 i = lastSize - 1; i >= 0; --i)
			{
				if (callback(Super::data[i]))
				{
					RemoveAtUnsafe(i, false);
				}
			}

			if (shouldShrink)
			{
				Shrink();
			}
			return lastSize - Super::size;
		}

		/**
		 * Delete all items that match a delegate
		 * @return number of deleted items
		 */
		i32 RemoveIfSwap(TFunction<bool(const Type&)>&& callback, const bool shouldShrink = true)
		{
			const i32 lastSize = Super::size;
			for (i32 i = lastSize - 1; i >= 0; --i)
			{
				if (callback(Super::data[i]))
				{
					RemoveAtSwapUnsafe(i);
				}
			}

			// First item is checked last to prevent invalid swap
			if (Super::size > 0 && callback(Super::data[0]))
			{
				if (Super::size > 1)
				{
					Super::SwapUnsafe(0, Super::size - 1);
				}
				RemoveLast();
			}

			if (shouldShrink)
			{
				Shrink();
			}
			return lastSize - Super::size;
		}

		void RemoveLast()
		{
			if (1 <= Super::size)
			{
				RemoveAtUnsafe(Super::size - 1);
			}
		}

		/**
		 * Remove N last elements from the array
		 * @param count
		 */
		void RemoveLast(i32 count)
		{
			if (count <= Super::size)
			{
				RemoveAtUnsafe(Super::size - count, count);
			}
		}

		/** Remove all elements from the array and optionally free memory
		 * @param shouldShrink true will free used memory
		 */
		void Clear(bool shouldShrink = true)
		{
			DestroyItems(Super::data, Super::size);
			Super::size = 0;
			if (shouldShrink)
			{
				Shrink();
			}
		}
#pragma endregion Removals


#pragma region Storage
	public:

		void Reserve(i32 newCapacity)
		{
			if (capacity < newCapacity)
			{
				Reallocate(newCapacity);
			}
		}
		void ReserveMore(i32 extraCapacity)
		{
			Reserve(Super::size + extraCapacity);
		}

		void Resize(i32 newSize)
		{
			if (newSize < Super::size)    // Trim
			{
				RemoveLast(Super::size - newSize);
			}
			else if (newSize > Super::size)    // Append
			{
				Append(newSize - Super::size);
			}
			// If size doens't change, do nothing
		}

		void Resize(i32 newSize, const Type& value)
		{
			if (newSize < Super::size)    // Trim
			{
				RemoveLast(Super::size - newSize);
			}
			else if (newSize > Super::size)    // Append
			{
				Append(newSize - Super::size, value);
			}
			// If size doens't change, do nothing
		}

		void Shrink(i32 minCapacity = 0)
		{
			// Reallocate will take care of not shrinking under size for us
			if (capacity > minCapacity)
			{
				Reallocate(minCapacity);
			}
		}

		i32 Capacity() const
		{
			return capacity;
		}


		/** @return true if this array has any capacity for inline data */
		static consteval bool HasInlineBuffer()
		{
			return InlineCapacity > 0;
		}

		/** @return true is inline data is being used as the buffer */
		constexpr bool UsesInlineBuffer() const
		{
			if constexpr (HasInlineBuffer())
			{
				return Super::data == inlineBuffer.Data();
			}
			return false;
		}

		constexpr Type* GetInlineBuffer() const
		{
			if constexpr (HasInlineBuffer())
			{
				return inlineBuffer.Data();
			}
			return nullptr;
		}

	protected:

		constexpr i32 GetGrownCapacity(i32 newSize)
		{
			static constexpr double exponentialFactor = 1.6;
			const i32 desiredCapacity                 = i32(capacity * exponentialFactor);
			return p::math::Max(desiredCapacity, newSize);
		}

		void GrowEnough(i32 newSize)
		{
			if (capacity < newSize)
			{
				Reallocate(GetGrownCapacity(newSize));
			}
		}

		/** @return true if element data needs to be moved */
		bool AllocNewBuffer(i32 newCapacity)
		{
			Check(capacity != newCapacity);

			// Can never reallocate to under our used size
			if (Super::size > newCapacity) [[unlikely]]
			{
				newCapacity = Super::size;
			}

			if (newCapacity > InlineCapacity)
			{
				Super::data = p::Alloc<Type>(*arena, newCapacity);
				capacity    = newCapacity;
				return true;
			}

			if constexpr (HasInlineBuffer())
			{
				if (newCapacity > 0)
				{
					Type* oldData = Super::data;
					Super::data   = inlineBuffer.Data();
					capacity      = InlineCapacity;
					return oldData != inlineBuffer.Data();
				}
			}

			Super::data = nullptr;
			capacity    = 0;
			return false;
		}

		void FreeOldBuffer(Type* oldData, const i32 oldCapacity)
		{
			if (!HasInlineBuffer() || oldData != inlineBuffer.Data())
			{
				// Only free memory if we were not using the inline array
				Free(*arena, oldData, oldCapacity);
			}
		}

		void Reallocate(i32 newCapacity)
		{
			Type* oldData         = Super::data;
			const i32 oldCapacity = capacity;
			if (AllocNewBuffer(newCapacity))    // Buffer changed
			{
				MoveOrCopyConstructItems<Type, true>(Super::data, Super::size, oldData);
			}
			FreeOldBuffer(oldData, oldCapacity);
		}

		Arena& GetArena() const
		{
			return *arena;
		}
#pragma endregion Storage


	protected:
		void AddUninitialized(i32 count);
		void InsertUninitialized(i32 atIndex, i32 count);
		void CopyFrom(const IArray<const Type>& other);

		template<u32 OtherInlineCapacity>
		void MoveFrom(TInlineArray<Type, OtherInlineCapacity>&& other);
	};


	template<typename Type>
	struct TView : public IArray<Type>
	{
		using Super = IArray<Type>;

		constexpr TView() = default;
		constexpr TView(Type& value)
		{
			Super::data = &value;
			Super::size = 1;
		}
		constexpr TView(Type* first, Type* last)
		{
			Super::data = first;
			Super::size = i32(last - first);
		}
		constexpr TView(Type* inData, i32 inSize)
		{
			Super::data = inData;
			Super::size = inSize;
		}

		template<i32 N>
		constexpr TView(Type (&value)[N])
		{
			Super::data = value;
			Super::size = N;
		}
		constexpr TView(std::initializer_list<Type> value)
		{
			Super::data = value.begin();
			Super::size = i32(value.size());
		}

		constexpr TView(const IArray<Type>& other)
		{
			Super::data = other.Data();
			Super::size = other.Size();
		}
		constexpr TView(const IArray<Mut<Type>>& other) requires(IsConst<Type>)
		{
			Super::data = other.Data();
			Super::size = other.Size();
		}
		constexpr TView(const IArray<Type>& value, i32 firstN)
		{
			Super::data = value.data;
			Super::size = math::Min(value.size, firstN);
		}
		constexpr TView(const IArray<Mut<Type>>& value, i32 firstN) requires(IsConst<Type>)
		{
			Super::data = value.data;
			Super::size = math::Min(value.size, firstN);
		}
		constexpr TView& operator=(const IArray<Type>& other)
		{
			Super::data = other.data;
			Super::size = other.size;
			return *this;
		}
		constexpr TView& operator=(const IArray<Mut<Type>>& other) requires(IsConst<Type>)
		{
			Super::data = other.data;
			Super::size = other.size;
			return *this;
		}
	};


	////////////////////////////////
	// DECLARATIONS
	//

	template<typename Type, u32 InlineCapacity>
	void TInlineArray<Type, InlineCapacity>::AddUninitialized(i32 count)
	{
		CheckMsg(count >= 0, "Cant add less than zero elements.");
		const i32 newSize = Super::size + count;
		GrowEnough(newSize);
		Super::size = newSize;
	}

	template<typename Type, u32 InlineCapacity>
	void TInlineArray<Type, InlineCapacity>::InsertUninitialized(i32 atIndex, i32 count)
	{
		const i32 oldSize = Super::size;
		const i32 newSize = Super::size + count;
		Super::size       = newSize;

		if (newSize > capacity)    // Reallocate elements
		{
			Type* oldData         = Super::data;
			const i32 oldCapacity = capacity;
			if (AllocNewBuffer(GetGrownCapacity(newSize)))    // Buffer changed
			{
				// Move elements before insertion index
				MoveOrCopyConstructItems<Type, true>(Super::data, atIndex, oldData);

				// Move elements after insertion index
				const i32 countToPush = oldSize - atIndex;
				MoveOrCopyConstructItems<Type, true>(
				    Super::data + atIndex + count, countToPush, oldData + atIndex);
			}
			FreeOldBuffer(oldData, oldCapacity);
		}
		else if (atIndex != Super::size)
		{
			Type* const ptrToPush = Super::data + atIndex;
			const i32 countToPush = oldSize - atIndex;
			// Push elements after insertion index
			MoveItemsBackwards(ptrToPush + count, countToPush, ptrToPush);
		}
		else
		{
			// If we insert at end we have nothing else to do
		}
	}

	template<typename Type, u32 InlineCapacity>
	void TInlineArray<Type, InlineCapacity>::CopyFrom(const IArray<const Type>& other)
	{
		Clear(false);
		Reserve(other.Size());
		Super::size = other.Size();
		CopyConstructItems<Type>(Super::data, Super::size, other.Data());
	}

	template<typename Type, u32 InlineCapacity>
	template<u32 OtherInlineCapacity>
	void TInlineArray<Type, InlineCapacity>::MoveFrom(
	    TInlineArray<Type, OtherInlineCapacity>&& other)
	{
		if (other.UsesInlineBuffer())    // We can't move from an inline buffer, so we move items
		{
			Clear(false);
			Super::size = other.size;

			Type* oldData         = Super::data;
			const i32 oldCapacity = capacity;
			if (AllocNewBuffer(GetGrownCapacity(Super::size)))    // Buffer changed
			{
				MoveOrCopyConstructItems<Type, true>(Super::data, Super::size, other.Data());
			}
			FreeOldBuffer(oldData, oldCapacity);
			other.Clear();
		}
		else
		{
			Clear(true);
			Super::data = other.data;
			Super::size = other.size;
			capacity    = other.capacity;
			arena       = other.arena;    // We pass the arena so that it can be correctly freed

			other.data     = nullptr;
			other.size     = 0;
			other.capacity = 0;
		}
	}
};    // namespace p
