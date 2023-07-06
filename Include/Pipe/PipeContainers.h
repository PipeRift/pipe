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

#include <initializer_list>
#include <iterator>


namespace p
{
	////////////////////////////////
	// FORWARD DECLARATIONS
	//

	template<typename Type>
	struct IRange;

	template<typename Type>
	struct TArray;

	template<typename Type, u32 InlineCapacity>
	struct TInlineArray;


	////////////////////////////////
	// RANGES
	//

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


	/** Containers inheriting IRange point to a contiguous list of elements of a type.
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


		// Do not use IRange as a container itself. See TView, TArray and TInlineArray.
		IRange() = default;

	public:
		Type* Data() const
		{
			return data;
		}

		i32 Size() const
		{
			return size;
		}

		bool IsEmpty() const
		{
			return size == 0;
		}

		bool IsValidIndex(i32 index) const
		{
			return index >= 0 && index < size;
		}
		bool IsValidIndex(u32 index) const
		{
			return index < u32(size);
		}

		i32 GetMemorySize() const
		{
			return size * sizeof(Type);
		}

		/// @returns a pointer to an element given a valid index, otherwise null
		Type* At(i32 index) const
		{
			return IsValidIndex(index) ? data + index : nullptr;
		}

		/**
		 * Array bracket operator. Returns reference to element at give index.
		 * @returns reference to indexed element.
		 */
		Type& operator[](i32 index) const
		{
			Check(IsValidIndex(index));
			return data[index];
		}

		Type& First() const
		{
			Check(size != 0);
			return data[0];
		}
		Type& Last() const
		{
			Check(size != 0);
			return data[size - 1];
		}


#pragma region Sort
		void Swap(i32 firstIndex, i32 secondIndex)
		{
			if (IsValidIndex(firstIndex) && IsValidIndex(secondIndex) && firstIndex != secondIndex)
			{
				std::iter_swap(data + firstIndex, data + secondIndex);
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
		void SortRange(i32 firstIndex, i32 count, Predicate predicate)
		{
			const i32 maxSize = Size() - firstIndex;
			p::Sort(data + firstIndex, math::Min(count, maxSize), TLess<Type>());
		}

		void SortRange(i32 firstIndex, i32 count)
		{
			SortRange(firstIndex, count, TLess<Type>());
		}
#pragma endregion Sort


#pragma region Search
		Iterator FindIt(const Type& value) const
		{
			return std::find(begin(), end(), value);
		}

		Iterator FindIt(TFunction<bool(const Type&)> cb) const
		{
			return std::find_if(begin(), end(), cb);
		}

		i32 FindIndex(const Type& value) const
		{
			ConstIterator found = FindIt(value);
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
		bool operator==(const IRange<OtherType>& other) const
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


		constexpr Iterator begin()
		{
			return Iterator(data, this);
		};
		constexpr ConstIterator begin() const
		{
			return ConstIterator(data, this);
		};
		constexpr Iterator end()
		{
			return Iterator(data + size, this);
		};
		constexpr ConstIterator end() const
		{
			return ConstIterator(data + size, this);
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


	////////////////////////////////
	// ARRAYS
	//

	/** An array type with a fixed amount of elements. */
	template<typename Type, u32 InlineCapacity>
	struct TInlineArray : public IRange<Type>
	{
	public:
		template<typename OtherType, u32 OtherInlineCapacity>
		friend struct TInlineArray;

		i32 capacity = 0;
		Arena* arena = &p::GetCurrentArena();
		mutable TTypeAsBytes<Type>
		    inlineBuffer[InlineCapacity];    // Not used if InlineCapacity is 0


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
		template<u32 OtherInlineCapacity>
		TInlineArray(const TInlineArray<Type, OtherInlineCapacity>& other)
		{
			CopyFrom(other);
		}
		template<u32 OtherInlineCapacity>
		TInlineArray<Type, InlineCapacity>& operator=(
		    const TInlineArray<Type, OtherInlineCapacity>& other)
		{
			if (this != (void*)&other)
			{
				CopyFrom(other);
			}
			return *this;
		}
		TInlineArray(const IRange<Type>& other)
		{
			CopyFrom(other);
		}
		TInlineArray<Type, InlineCapacity>& operator=(const IRange<Type>& other)
		{
			if (this != (void*)&other)
			{
				CopyFrom(other);
			}
			return *this;
		}
#pragma endregion Constructors


#pragma region Insertions
	public:
		i32 Add()
		{
			const i32 firstIndex = size;
			AddUninitialized(1);
			new (data + firstIndex) Type();
			return firstIndex;
		}
		i32 Add(Type&& value)
		{
			const i32 firstIndex = size;
			AddUninitialized(1);
			new (data + firstIndex) Type(Forward<Type>(value));
			return firstIndex;
		}
		i32 Add(const Type& value)
		{
			const i32 firstIndex = size;
			AddUninitialized(1);
			new (data + firstIndex) Type(value);
			return firstIndex;
		}
		Type& AddRef()
		{
			return data[Add()];
		}
		Type& AddRef(Type&& value)
		{
			return data[Add(Forward<Type>(value))];
		}
		Type& AddRef(const Type& value)
		{
			return data[Add(value)];
		}

		i32 FindOrAdd(const Type& value) const
		{
			const i32 found = FindIndex(value);
			if (found != NO_INDEX)
			{
				return found;
			}
			return Add(value);
		}

		/** Finds or adds an element in a sorted array.
		 * Much more efficient that FindOrAdd.
		 * NOTE: Undefined behavior on unsorted arrays!
		 * @param value to find or add
		 * @param sortPredicate used to sort the array
		 * @param insertSorted when true(default) inserts element sorted
		 * @return index, added
		 */
		template<typename SortPredicate = TLess<Type>>
		i32 FindOrAddSorted(const Type& value, SortPredicate sortPredicate = {},
		    bool insertSorted = true, bool* outFound = nullptr)
		{
			const i32 index = LowerBound(value, sortPredicate);
			if (index != NO_INDEX)
			{
				if (!sortPredicate(value, data[index]))    // Equal check, found element
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
		i32 FindOrAddSorted(Type&& value, SortPredicate sortPredicate = {},
		    bool insertSorted = true, bool* outFound = nullptr)
		{
			const i32 index = LowerBound(value, sortPredicate);
			if (index != NO_INDEX)
			{
				if (!sortPredicate(value, data[index]))    // Equal check, found element
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
			const i32 firstIndex = size;
			AddUninitialized(count);
			ConstructItems(data + firstIndex, count);
		}
		void Append(i32 count, const Type& value)
		{
			const i32 firstIndex = size;
			AddUninitialized(count);
			ConstructItems(data + firstIndex, count, value);
		}
		void Append(const Type* values, i32 count)
		{
			const i32 firstIndex = size;
			AddUninitialized(count);
			CopyConstructItems(data + firstIndex, count, values);
		}
		void Append(const IRange<Type>& values)
		{
			Append(values.Data(), values.Size());
		}
		void Append(std ::initializer_list<Type> initList)
		{
			Append(initList.begin(), i32(initList.size()));
		}

		void Assign(i32 count)
		{
			Clear(false);
			Reserve(count);
			size = count;
			ConstructItems(data, count);
		}
		void Assign(i32 count, const Type& value)
		{
			Clear(false);
			Reserve(count);
			size = count;
			ConstructItems(data, count, value);
		}
		void Assign(const Type* values, i32 count)
		{
			Clear(false);
			Reserve(count);
			size = count;
			CopyConstructItems(data, count, values);
		}
		void Assign(const IRange<Type>& values)
		{
			Assign(values.Data(), values.Size());
		}
		void Assign(std ::initializer_list<Type> initList)
		{
			Assign(initList.begin(), i32(initList.size()));
		}

		// TODO
		void Insert(i32 atIndex, Type&& value)
		{
			InsertUninitialized(atIndex, count);
			MoveConstructItems(data + atIndex, count, Forward<Type>(value));
		}
		void Insert(i32 atIndex, const Type& value)
		{
			InsertUninitialized(atIndex, count);
			CopyConstructItems(data + atIndex, count, value);
		}
		void Insert(i32 atIndex, const Type* values, i32 count)
		{
			InsertUninitialized(atIndex, count);
			CopyConstructItems(data + atIndex, count, values);
		}
		void Insert(i32 atIndex, const IRange<Type>& values)
		{
			Insert(atIndex, values.Data(), values.Size());
		}
		void Insert(i32 atIndex, std ::initializer_list<Type> initList)
		{
			Insert(atIndex, initList.data(), i32(initList.size()));
		}
#pragma endregion Insertions


#pragma region Removals
	public:
		void RemoveLast()
		{
			if (size > 0)
			{
				DestroyItems(data + size - 1, 1);
				--size;
			}
		}

		/**
		 * Remove N last elements from the array
		 * @param count
		 */
		void RemoveLast(i32 count)
		{
			if (size < count)
			{
				Clear(false);
			}
			else if (count > 0)
			{
				DestroyItems(data + size - count, count);
				size -= count;
			}
		}

		/** Remove all elements from the array and optionally free memory
		 * @param shouldShrink true will free used memory
		 */
		void Clear(bool shouldShrink = true)
		{
			DestroyItems(data, size);
			size = 0;
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
			Reserve(size + extraCapacity);
		}

		void Resize(i32 newSize)
		{
			if (newSize < size)    // Trim
			{
				RemoveLast(size - newSize);
			}
			else if (newSize > size)    // Append
			{
				Append(newSize - size);
			}
			// If size doens't change, do nothing
		}

		void Resize(i32 newSize, const Type& value)
		{
			if (newSize < size)    // Trim
			{
				RemoveLast(size - newSize, value);
			}
			else if (newSize > size)    // Append
			{
				Append(newSize - size, value);
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
				return data == inlineBuffer->AsType();
			}
			return false;
		}

		constexpr Type* GetInlineBuffer() const
		{
			if constexpr (HasInlineBuffer())
			{
				return inlineBuffer->AsType();
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
			if (size > newCapacity) [[unlikely]]
			{
				newCapacity = size;
			}

			if (newCapacity > InlineCapacity)
			{
				data     = p::Alloc<Type>(*arena, newCapacity);
				capacity = newCapacity;
				return true;
			}

			if constexpr (HasInlineBuffer())
			{
				if (newCapacity > 0)
				{
					Type* oldData = data;
					data          = inlineBuffer->AsType();
					capacity      = InlineCapacity;
					return oldData != inlineBuffer->AsType();
				}
			}

			data     = nullptr;
			capacity = 0;
			return false;
		}

		void FreeOldBuffer(Type* oldData, const i32 oldCapacity)
		{
			if (HasInlineBuffer() && oldData != inlineBuffer->AsType())
			{
				// Only free memory if we were not using the inline array
				Free(*arena, oldData, oldCapacity);
			}
		}

		void Reallocate(i32 newCapacity)
		{
			Type* oldData         = data;
			const i32 oldCapacity = capacity;
			if (AllocNewBuffer(newCapacity))    // Buffer changed
			{
				MoveOrCopyConstructItems<Type, true>(data, size, oldData);
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
		void CopyFrom(const IRange<Type>& other);

		template<u32 OtherInlineCapacity>
		void MoveFrom(TInlineArray<Type, OtherInlineCapacity>&& other);
	};


	template<typename Type>
	using TArray2 = TInlineArray<Type, 0>;

	template<typename T>
	using TSmallArray = TInlineArray<Type, 5>;


	////////////////////////////////
	// DECLARATIONS
	//

	template<typename Type, u32 InlineCapacity>
	void TInlineArray<Type, InlineCapacity>::AddUninitialized(i32 count)
	{
		CheckMsg(count >= 0, "Cant add less than zero elements.");
		const i32 newSize = size + count;
		GrowEnough(newSize);
		size = newSize;
	}

	template<typename Type, u32 InlineCapacity>
	void TInlineArray<Type, InlineCapacity>::InsertUninitialized(i32 atIndex, i32 count)
	{
		const i32 oldSize = size;
		const i32 newSize = size + count;
		size              = newSize;

		if (newSize > capacity)
		{
			// Reallocate elements
			Type* oldData         = data;
			const i32 oldCapacity = capacity;
			if (AllocNewBuffer(GetGrownCapacity(newSize)))    // Buffer changed
			{
				const i32 countBefore = atIndex;
				const i32 countAfter  = oldSize - atIndex;

				// Move elements before insertion index
				MoveOrCopyConstructItems<Type, true>(data, countBefore, oldData);

				// Move elements after insertion index
				MoveOrCopyConstructItems<Type, true>(
				    data + atIndex + count, countAfter, oldData + atIndex);
			}
			FreeOldBuffer(oldData, oldCapacity);
		}
		else
		{
			// Move trailing elements back
		}
	}

	template<typename Type, u32 InlineCapacity>
	void TInlineArray<Type, InlineCapacity>::CopyFrom(const IRange<Type>& other)
	{
		Clear(false);
		Reserve(other.Size());
		size = other.Size();
		CopyConstructItems<Type>(data, size, other.Data());
	}

	template<typename Type, u32 InlineCapacity>
	template<u32 OtherInlineCapacity>
	void TInlineArray<Type, InlineCapacity>::MoveFrom(
	    TInlineArray<Type, OtherInlineCapacity>&& other)
	{
		if (other.UsesInlineBuffer())    // We can't move from an inline buffer, so we move items
		{
			Clear(false);
			size = other.size;

			Type* oldData         = data;
			const i32 oldCapacity = capacity;
			if (AllocNewBuffer(GetGrownCapacity(size)))    // Buffer changed
			{
				MoveOrCopyConstructItems<Type, true>(data, size, other.Data());
			}
			FreeOldBuffer(oldData, oldCapacity);
			other.Clear();
		}
		else
		{
			Clear(true);
			data     = other.data;
			size     = other.size;
			capacity = other.capacity;
			arena    = other.arena;    // We pass the arena so that it can be correctly freed

			other.data     = nullptr;
			other.size     = 0;
			other.capacity = 0;
		}
	}
};    // namespace p
