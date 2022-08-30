// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Hash.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Memory/STLAllocator.h"

#include <tsl/sparse_set.h>

#include <cassert>
#include <type_traits>


namespace p::core
{
	template<typename Type, typename Allocator = ArenaAllocator>
	class TSet
	{
		// static_assert(std::is_nothrow_move_constructible<Value>::value ||
		//                  std::is_copy_constructible<Value>::value,
		//    "Value type must be nothrow move constructible and/or copy constructible.");

	public:
		template<typename OtherType, typename OtherAllocator>
		friend class TSet;

		using AllocatorType = Allocator;
		using HashSetType =
		    tsl::sparse_set<Type, Hash<Type>, std::equal_to<Type>, STLAllocator<Type, Allocator>>;

		using Iterator      = typename HashSetType::iterator;
		using ConstIterator = typename HashSetType::const_iterator;


	private:
		HashSetType set;


	public:
		TSet() = default;
		TSet(u32 defaultSize) : set(defaultSize) {}
		TSet(const Type& item) : set{}
		{
			Insert(item);
		}
		TSet(std::initializer_list<Type> initList) : set{initList.begin(), initList.end()} {}

		TSet(TSet&& other) noexcept            = default;
		TSet(const TSet& other)                = default;
		TSet& operator=(TSet&& other) noexcept = default;
		TSet& operator=(const TSet& other)     = default;

		Iterator Insert(Type&& value)
		{
			return set.insert(Move(value)).first;
		}

		Iterator Insert(const Type& value)
		{
			return set.insert(value).first;
		}

		void Append(const TSet<Type>& other)
		{
			if (other.Size() > 0)
			{
				if (Size() <= 0)
				{
					CopyFrom(other);
				}
				else
				{
					set.insert(other.begin(), other.end());
				}
			}
		}

		void Append(TSet<Type>&& other)
		{
			if (other.Size() > 0)
			{
				if (Size() <= 0)
				{
					MoveFrom(Move(other));
				}
				else
				{
					set.insert(set.end(), other.begin(), other.end());
				}
			}
		}

		void Resize(i32 sizeNum)
		{
			set.reserve(sizeNum);
		}

		template<typename Key = Type>
		Iterator FindIt(const Key& key)
		{
			return set.find(key);
		}

		template<typename Key = Type>
		Iterator FindIt(const Key& key, sizet hash)
		{
			return set.find(key, hash);
		}

		template<typename Key = Type>
		ConstIterator FindIt(const Key& key) const
		{
			return set.find(key);
		}

		template<typename Key = Type>
		ConstIterator FindIt(const Key& key, sizet hash) const
		{
			return set.find(key, hash);
		}

		template<typename Key = Type>
		Type* Find(const Key& key) const
		{
			ConstIterator it = FindIt(key);
			return it != end() ? &const_cast<Type&>(*it) : nullptr;
		}

		template<typename Key = Type>
		Type* Find(const Key& key, sizet hash) const
		{
			ConstIterator it = FindIt(key, hash);
			return it != end() ? &const_cast<Type&>(*it) : nullptr;
		}

		template<typename Key = Type>
		Type& FindRef(const Key& key) const
		{
			ConstIterator it = FindIt(key);
			assert(it != end() && "Value not found, can't dereference its value");
			return const_cast<Type&>(*it);
		}

		template<typename Key = Type>
		Type& FindRef(const Key& key, sizet hash) const
		{
			ConstIterator it = FindIt(key, hash);
			assert(it != end() && "Value not found, can't dereference its value");
			return const_cast<Type&>(*it);
		}

		bool Contains(const Type& value) const
		{
			return FindIt(value) != set.end();
		}

		/**
		 * Delete all items that match another provided item
		 * @return number of deleted items
		 */
		i32 Remove(const Type& value)
		{
			return RemoveIt(FindIt(value));
		}

		i32 RemoveIt(Iterator it)
		{
			if (it != end())
			{
				const i32 lastSize = Size();
				set.erase(it);
				return lastSize - Size();
			}
			return 0;
		}

		/** Empty the array.
		 * @param shouldShrink false will not free memory
		 */
		void Clear(const bool shouldShrink = true, i32 sizeNum = 0)
		{
			if (shouldShrink)
			{
				set.clear();
			}
			else
			{
				set.clear_no_resize();
				if (sizeNum > 0 && set.max_size() != sizeNum)
				{
					set.reserve(sizeNum);
				}
			}
		}

		void Reserve(u32 size)
		{
			set.reserve(size);
		}

		void Rehash(u32 size)
		{
			set.rehash(size);
		}

		i32 Size() const
		{
			return i32(set.size());
		}

		bool IsValidIndex(i32 index) const
		{
			return index >= 0 && index < Size();
		}


		/** OPERATORS */
	public:
		/**
		 * Array bracket operator. Returns reference to value at given value.
		 *
		 * @returns Reference to indexed element.
		 */
		const Type& operator[](const Type& value) const
		{
			return FindRef(value);
		}

		// Iterator functions
		Iterator begin()
		{
			return set.begin();
		};
		ConstIterator begin() const
		{
			return set.begin();
		};
		ConstIterator cbegin() const
		{
			return set.cbegin();
		};

		Iterator end()
		{
			return set.end();
		};
		ConstIterator end() const
		{
			return set.end();
		};
		ConstIterator cend() const
		{
			return set.cend();
		};


		/** INTERNAL */
	private:
		void CopyFrom(const TSet<Type>& other)
		{
			set = other.set;
		}

		void MoveFrom(TSet<Type>&& other)
		{
			set = Move(other.set);
		}
	};
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
