// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Events/Function.h"
#include "Math/Search.h"
#include "Math/Sorting.h"
#include "Memory/STLAllocator.h"
#include "Platform/Platform.h"
#include "Templates/Less.h"
#include "Templates/Tuples.h"
#include "TypeTraits.h"

#include <cassert>
#include <vector>


namespace Rift
{
	template<typename Type, typename Allocator = Memory::DefaultAllocator>
	class TArray
	{
	public:
		template<typename OtherType, typename OtherAllocator>
		friend class TArray;

		using ItemType   = Type;
		using VectorType = std::vector<Type, STLAllocator<Type, Allocator>>;

		using Iterator             = typename VectorType::iterator;
		using ConstIterator        = typename VectorType::const_iterator;
		using ReverseIterator      = typename VectorType::reverse_iterator;
		using ConstReverseIterator = typename VectorType::const_reverse_iterator;


	private:
		VectorType vector{};


	public:
		TArray() = default;

		TArray(u32 defaultSize) : vector(defaultSize) {}
		TArray(u32 defaultSize, const Type& defaultValue) : vector(defaultSize, defaultValue) {}
		TArray(std::initializer_list<Type> initList) : vector{initList} {}
		TArray(const Type* data, i32 sizeNum)
		{
			Assign(data, sizeNum);
		}

		TArray(TArray<Type>&& other)
		{
			MoveFrom(Move(other));
		}
		TArray<Type>& operator=(TArray<Type>&& other)
		{
			MoveFrom(Move(other));
			return *this;
		}

		TArray(const TArray<Type>& other)
		{
			CopyFrom(other);
		}
		TArray<Type>& operator=(const TArray<Type>& other)
		{
			CopyFrom(other);
			return *this;
		}

		i32 Add(Type&& item)
		{
			vector.push_back(Move(item));
			return Size() - 1;
		}

		i32 Add(const Type& item)
		{
			vector.push_back(item);
			return Size() - 1;
		}

		Type& AddRef(Type&& item)
		{
			const i32 index = Add(Move(item));
			return Data()[index];
		}

		Type& AddRef(const Type& item)
		{
			const i32 index = Add(item);
			return Data()[index];
		}

		i32 AddUnique(const Type& item)
		{
			const i32 foundIndex = FindIndex(item);
			if (foundIndex == NO_INDEX)
			{
				return Add(item);
			}
			return foundIndex;
		}

		i32 AddDefaulted()
		{
			vector.push_back({});
			return Size() - 1;
		}

		Type& AddDefaultedRef()
		{
			const i32 index = AddDefaulted();
			return Data()[index];
		}

		template<typename SortPredicate = TLess<Type>>
		i32 AddSorted(Type&& item, SortPredicate sortPredicate = {})
		{
			const i32 index = LowerBound(item, sortPredicate);
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
			const i32 index = LowerBound(item, sortPredicate);
			if (index != NO_INDEX)
			{
				Insert(index, item);
				return index;
			}
			return Add(item);
		}

		void Append(const TArray<Type>& other)
		{
			if (other.Size() > 0)
			{
				if (Size() <= 0)
					CopyFrom(other);
				else
					vector.insert(vector.end(), other.begin(), other.end());
			}
		}

		void Append(TArray<Type>&& other)
		{
			if (other.Size() > 0)
			{
				if (Size() <= 0)
					MoveFrom(Move(other));
				else
					vector.insert(vector.end(), other.begin(), other.end());
			}
		}

		template<typename Container>    // Intended for TSpan<T>
		void Append(Container values)
		{
			vector.insert(vector.end(), values.begin(), values.end());
		}

		void Reserve(i32 sizeNum)
		{
			vector.reserve(sizeNum);
		}
		void ReserveMore(i32 sizeNum)
		{
			Reserve(Size() + sizeNum);
		}
		void Resize(i32 sizeNum)
		{
			vector.resize(sizeNum);
		}

		void Assign(i32 sizeNum, const Type& value)
		{
			vector.assign(sizeNum, value);
		}

		void Assign(const Type* data, i32 sizeNum)
		{
			vector.assign(data, data + sizeNum);
		}

		void AssignAll(const Type& value)
		{
			Assign(Size(), value);
		}

		void Insert(i32 index, Type&& item)
		{
			vector.insert(vector.begin() + index, Move(item));
		}

		void Insert(i32 index, const Type& item, i32 count = 1)
		{
			if (IsValidIndex(index))
			{
				if (count == 1)
					vector.insert(vector.begin() + index, item);
				else
					vector.insert(vector.begin() + index, count, item);
			}
		}

		template<typename Container>    // Intended for TSpan<T>
		void InsertRange(i32 index, Container values)
		{
			if (IsValidIndex(index))
			{
				vector.insert(vector.begin() + index, values.begin(), values.end());
			}
			else
			{
				Append(values);
			}
		}

		void InsertDefaulted(i32 index, i32 count = 1)
		{
			Insert(index, {}, count);
		}


		template<typename Predicate>
		void Sort(Predicate predicate)
		{
			Algorithms::Sort(Data(), Size(), predicate);
		}

		void Sort()
		{
			Algorithms::Sort(Data(), Size(), TLess<Type>());
		}

		Iterator FindIt(const Type& item) const
		{
			auto& nonConstVector = const_cast<VectorType&>(vector);
			return std::find(nonConstVector.begin(), nonConstVector.end(), item);
		}

		Iterator FindIt(TFunction<bool(const Type&)> cb) const
		{
			auto& nonConstVector = const_cast<VectorType&>(vector);
			return std::find_if(nonConstVector.begin(), nonConstVector.end(), cb);
		}

		i32 FindIndex(const Type& item) const
		{
			ConstIterator found = FindIt(item);
			if (found != vector.end())
			{
				return i32(std::distance(vector.begin(), found));
			}
			return NO_INDEX;
		}

		i32 FindIndex(TFunction<bool(const Type&)> cb) const
		{
			ConstIterator it = FindIt(Move(cb));
			if (it != vector.end())
			{
				return i32(std::distance(vector.begin(), it));
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

		i32 FindOrAdd(const Type& item) const
		{
			const i32 found = FindIndex(item);
			if (found != NO_INDEX)
			{
				return found;
			}
			return Add(item);
		}

		/** Finds or adds an element in a sorted array.
		 * Much more efficient that FindOrAdd.
		 * NOTE: Undefined behavior on unsorted arrays!
		 * @param item to find or add
		 * @param sortPredicate used to sort the array
		 * @param insertSorted when true(default) inserts element sorted
		 * @return index, added
		 */
		template<typename SortPredicate = TLess<Type>>
		TPair<i32, bool> FindOrAddSorted(
		    const Type& item, SortPredicate sortPredicate = {}, bool insertSorted = true)
		{
			const i32 index = LowerBound(item, sortPredicate);
			if (index != NO_INDEX)
			{
				if (!sortPredicate(item, Data()[index]))    // Equal check, found element
				{
					return {index, false};
				}
				else if (insertSorted)
				{
					Insert(index, item);
					return {index, true};
				}
			}
			return {Add(item), true};
		}

		template<typename SortPredicate = TLess<Type>>
		TPair<i32, bool> FindOrAddSorted(
		    Type&& item, SortPredicate sortPredicate = {}, bool insertSorted = true)
		{
			const i32 index = LowerBound(item, sortPredicate);
			if (index != NO_INDEX)
			{
				if (!sortPredicate(item, Data()[index]))    // Equal check, found element
				{
					return {index, false};
				}
				else if (insertSorted)
				{
					Insert(index, Move(item));
					return {index, true};
				}
			}
			return {Add(Move(item)), true};
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
			return Algorithms::LowerBound(Data(), 0, Size(), value, sortPredicate);
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
			return Algorithms::UpperBound(Data(), 0, Size(), value, sortPredicate);
		}

		template<typename Value, typename SortPredicate = TLess<>>
		i32 FindSortedEqual(const Value& value, SortPredicate sortPredicate = {}) const
		{
			return Algorithms::BinarySearch(Data(), 0, Size(), value, sortPredicate);
		}

		template<typename Value>
		i32 FindSortedMax(const Value& max, bool included = false) const
		{
			return Algorithms::FindSortedMax(Data(), 0, Size(), max, included);
		}

		template<typename Value>
		i32 FindSortedMin(const Value& min, bool included = false) const
		{
			return Algorithms::FindSortedMin(Data(), 0, Size(), min, included);
		}

		bool Contains(const Type& item) const
		{
			return FindIt(item) != vector.end();
		}

		bool Contains(TFunction<bool(const Type&)> cb) const
		{
			return FindIt(Move(cb)) != vector.end();
		}

		/**
		 * Delete all items that match another provided item
		 * @return number of deleted items
		 */
		i32 Remove(const Type& item, const bool shouldShrink = true)
		{
			const i32 lastSize = Size();
			(void)std::remove(vector.begin(), vector.end(), item);

			if (shouldShrink)
			{
				Shrink();
			}
			return lastSize - Size();
		}

		template<typename Container>    // Intended for TSpan<T>
		i32 RemoveMany(Container items, const bool shouldShrink = true)
		{
			const i32 lastSize = Size();
			for (i32 i = 0; i < Size(); ++i)
			{
				if (items.Contains(Data()[i]))
				{
					RemoveAtUnsafe(i, false);
					--i;    // Repeat same index
				}
			}
			if (shouldShrink)
			{
				Shrink();
			}
			return lastSize - Size();
		}

		// Removes an item assuming the array is sorted
		template<typename SortPredicate = TLess<>>
		i32 RemoveSorted(
		    const Type& item, SortPredicate sortPredicate = {}, const bool shouldShrink = true)
		{
			return i32(RemoveAt(FindSortedEqual(item, sortPredicate)));
		}

		/**
		 * Delete item at index
		 * @return true if removed
		 */
		bool RemoveAt(i32 index, const bool shouldShrink = true)
		{
			if (IsValidIndex(index))
			{
				return RemoveAtUnsafe(index, shouldShrink);
			}
			return false;
		}

		/**
		 * Delete item at index.
		 * Unsafe version. Doesn't make sure index is valid!
		 * @return true if removed
		 */
		bool RemoveAtUnsafe(i32 index, const bool shouldShrink = true)
		{
			const i32 lastSize = Size();
			vector.erase(vector.begin() + index);

			if (shouldShrink)
				Shrink();

			return lastSize - Size() > 0;
		}

		/**
		 * Delete item at an index by swapping with the last element. Doesn't preserve order.
		 * Faster than RemoveAt since it doesnt push all remaining elements 1 position left.
		 * @return true if removed
		 */
		bool RemoveAtSwap(i32 index, const bool shouldShrink = true)
		{
			if (IsValidIndex(index))
			{
				return RemoveAtSwapUnsafe(index, shouldShrink);
			}
			return false;
		}

		/**
		 * Delete item at an index by swapping with the last element. Doesn't preserve order.
		 * Faster than RemoveAt since it doesnt push all remaining elements 1 position left.
		 * Unsafe version. Doesn't make sure index is valid!
		 * @return true if removed
		 */
		bool RemoveAtSwapUnsafe(i32 index, const bool shouldShrink = true)
		{
			const i32 lastSize = Size();
			Swap(index, lastSize - 1);
			vector.pop_back();

			if (shouldShrink)
				Shrink();

			return lastSize - Size() > 0;
		}

		/**
		 * Delete all items that match a delegate
		 * @return number of deleted items
		 */
		i32 RemoveIf(TFunction<bool(const Type&)>&& callback, const bool shouldShrink = true)
		{
			const i32 lastSize = Size();
			// We remove from the back so that there are less elements to move when removing an
			// element
			for (i32 i = lastSize - 1; i >= 0; --i)
			{
				if (callback(Data()[i]))
				{
					RemoveAtUnsafe(i, false);
				}
			}

			if (shouldShrink)
			{
				Shrink();
			}
			return lastSize - Size();
		}

		/**
		 * Delete all items that match a delegate
		 * @return number of deleted items
		 */
		i32 RemoveIfSwap(TFunction<bool(const Type&)>&& callback, const bool shouldShrink = true)
		{
			const i32 lastSize = Size();
			for (i32 i = 0; i < Size(); ++i)
			{
				if (callback(Data()[i]))
				{
					RemoveAtSwapUnsafe(i, false);
					--i;    // We removed one item, so we iterate the same index
				}
			}

			if (shouldShrink)
			{
				Shrink();
			}
			return lastSize - Size();
		}

		bool RemoveLast(i32 num = 1)
		{
			const i32 lastSize = Size();
			Resize(lastSize - num);
			return lastSize - Size() > 0;
		}

		void Swap(i32 firstIndex, i32 secondIndex);

		/** Empty the array.
		 * @param shouldShrink false will not free memory
		 */
		void Empty(const bool shouldShrink = true, i32 sizeNum = 0)
		{
			vector.clear();

			if (shouldShrink)
				Shrink();
			else if (sizeNum > 0)
				Reserve(sizeNum);
		}

		void Shrink()
		{
			vector.shrink_to_fit();
		}

		i32 Size() const
		{
			return i32(vector.size());
		}

		// Return the size of the data used in bytes
		i32 GetDataSize()
		    const    // std::vector<bool> doesn't guarantee that this function is accurate
		    requires(!IsSame<Type, bool>)
		{
			return Size() * sizeof(Type);
		}

		i32 MaxSize() const
		{
			return (i32)vector.capacity();
		}

		bool IsEmpty() const
		{
			return Size() == 0;
		}

		bool IsValidIndex(i32 index) const
		{
			return index >= 0 && index < Size();
		}
		bool IsValidIndex(u32 index) const
		{
			return index < u32(Size());
		}

		Type& First()
		{
			return vector.front();
		}
		Type& Last()
		{
			return vector.back();
		}
		const Type& First() const
		{
			return vector.front();
		}
		const Type& Last() const
		{
			return vector.back();
		}

		Type* Data() requires(!IsSame<Type, bool>)    // std::vector of bool can't access Data()
		{
			return vector.data();
		}
		const Type* Data() const
		    requires(!IsSame<Type, bool>)    // std::vector of bool can't access Data()
		{
			return vector.data();
		}

		/** OPERATORS */
	public :
		/**
		 * Array bracket operator. Returns reference to element at give index.
		 *
		 * @returns Reference to indexed element.
		 */
		Type&
		operator[](i32 index) requires(!IsSame<Type, bool>)
		{
			assert(IsValidIndex(index));
			return Data()[index];
		}

		/**
		 * Array bracket operator. Returns reference to element at give index.
		 *
		 * Const version of the above.
		 *
		 * @returns Reference to indexed element.
		 */
		const Type& operator[](i32 index) const requires(!IsSame<Type, bool>)
		{
			assert(IsValidIndex(index));
			return Data()[index];
		}

		typename VectorType::reference operator[](i32 index) requires(IsSame<Type, bool>)
		{
			assert(IsValidIndex(index));
			// std::vector of bool can't access Data()
			return vector.operator[](index);
		}
		typename VectorType::const_reference operator[](i32 index) const
		    requires(IsSame<Type, bool>)
		{
			assert(IsValidIndex(index));
			// std::vector of bool can't access Data()
			return vector.operator[](index);
		}

		Iterator begin()
		{
			return vector.begin();
		};
		ConstIterator begin() const
		{
			return vector.begin();
		};
		ConstIterator cbegin() const
		{
			return vector.cbegin();
		};

		Iterator end()
		{
			return vector.end();
		};
		ConstIterator end() const
		{
			return vector.end();
		};
		ConstIterator cend() const
		{
			return vector.cend();
		};

		ReverseIterator rbegin()
		{
			return vector.rbegin();
		};
		ConstReverseIterator rbegin() const
		{
			return vector.rbegin();
		};
		ConstReverseIterator crbegin() const
		{
			return vector.crbegin();
		};

		ReverseIterator rend()
		{
			return vector.rend();
		};
		ConstReverseIterator rend() const
		{
			return vector.rend();
		};
		ConstReverseIterator crend() const
		{
			return vector.crend();
		};

		/** INTERNAL */
	private:
		void CopyFrom(const TArray& other)
		{
			vector = other.vector;
		}
		void MoveFrom(TArray&& other)
		{
			vector = Move(other.vector);
		}
	};


	template<typename Type, typename Allocator>
	void TArray<Type, Allocator>::Swap(i32 firstIndex, i32 secondIndex)
	{
		if (Size() > 1 && firstIndex != secondIndex && IsValidIndex(firstIndex)
		    && IsValidIndex(secondIndex))
		{
			std::iter_swap(vector.begin() + firstIndex, vector.begin() + secondIndex);
		}
	}
}    // namespace Rift
