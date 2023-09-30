// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/FixedString.h"
#include "Pipe/Core/Hash.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/Templates.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/STLAllocator.h"

#include <tsl/sparse_map.h>

#include <cassert>
#include <type_traits>


namespace p::core
{
	template<typename Key, typename Value>
	class TMap
	{
		static_assert(std::is_nothrow_move_constructible<Value>::value
		                  || std::is_copy_constructible<Value>::value,
		    "Value type must be nothrow move constructible and/or copy constructible.");

	public:
		template<typename OtherKey, typename OtherValue>
		friend class TMap;

		using KeyType   = Key;
		using ValueType = Value;
		using KeyEqual  = std::equal_to<KeyType>;
		using Allocator = STLAllocator<TPair<Key, Value>>;
		using HashMapType =
		    tsl::sparse_map<KeyType, ValueType, HashResolver<KeyType>, KeyEqual, Allocator>;

		using Iterator      = typename HashMapType::iterator;
		using ConstIterator = typename HashMapType::const_iterator;

		template<typename U>
		using HasIsTransparent = tsl::detail_sparse_hash::has_is_transparent<U>;


	private:
		HashMapType map;


	public:
		TMap(Arena& arena = GetCurrentArena()) : map{Allocator{arena}} {}
		TMap(u32 defaultSize, Arena& arena = GetCurrentArena()) : map(defaultSize, Allocator{arena})
		{}
		TMap(const TPair<const KeyType, ValueType>& item, Arena& arena = GetCurrentArena())
		    : TMap(arena)
		{
			Insert(item);
		}
		TMap(std::initializer_list<TPair<const KeyType, ValueType>> initList,
		    Arena& arena = GetCurrentArena())
		    : map{initList.begin(), initList.end(), 0, Allocator{arena}}
		{}

		TMap(TMap&& other) noexcept            = default;
		TMap(const TMap& other)                = default;
		TMap& operator=(TMap&& other) noexcept = default;
		TMap& operator=(const TMap& other)     = default;

		Iterator Insert(KeyType&& key, ValueType&& value)
		{
			return map.insert({Move(key), Move(value)}).first;
		}

		Iterator Insert(const KeyType& key, ValueType&& value)
		{
			return map.insert({key, Move(value)}).first;
		}

		Iterator Insert(KeyType&& key, const ValueType& value)
		{
			return map.insert({Move(key), value}).first;
		}

		Iterator Insert(const KeyType& key, const ValueType& value)
		{
			return map.insert({key, value}).first;
		}

		Iterator Insert(const TPair<KeyType, ValueType>& pair)
		{
			return map.insert(pair).first;
		}

		Iterator InsertDefaulted(const KeyType& key)
		{
			return map.insert({key, ValueType{}}).first;
		}

		Iterator InsertDefaulted(KeyType&& key)
		{
			return map.insert({Move(key), ValueType{}}).first;
		}

		void Append(const TMap<KeyType, ValueType>& other)
		{
			if (other.Size() > 0)
			{
				if (Size() <= 0)
				{
					CopyFrom(other);
				}
				else
				{
					map.insert(other.begin(), other.end());
				}
			}
		}

		void Append(TMap<KeyType, ValueType>&& other)
		{
			if (other.Size() > 0)
			{
				if (Size() <= 0)
				{
					MoveFrom(Move(other));
				}
				else
				{
					map.insert(map.end(), other.begin(), other.end());
				}
			}
		}

		void Resize(i32 sizeNum)
		{
			// map.resize(sizeNum);
		}

		Iterator FindIt(const KeyType& key)
		{
			return map.find(key);
		}

		Iterator FindIt(const KeyType& key, sizet precalculatedHash)
		{
			return map.find(key, precalculatedHash);
		}

		ConstIterator FindIt(const KeyType& key) const
		{
			return map.find(key);
		}

		ConstIterator FindIt(const KeyType& key, sizet precalculatedHash) const
		{
			return map.find(key, precalculatedHash);
		}

		template<typename K>
		Iterator FindIt(const K& key) requires(HasIsTransparent<KeyEqual>::value)
		{
			return map.find(key);
		}

		template<typename K>
		Iterator FindIt(const K& key, sizet precalculatedHash)
		    requires(HasIsTransparent<KeyEqual>::value)
		{
			return map.find(key, precalculatedHash);
		}

		template<typename K>
		ConstIterator FindIt(const K& key) const requires(HasIsTransparent<KeyEqual>::value)
		{
			return map.find(key);
		}

		template<typename K>
		ConstIterator FindIt(const K& key, sizet precalculatedHash) const
		    requires(HasIsTransparent<KeyEqual>::value)
		{
			return map.find(key, precalculatedHash);
		}

		template<typename K>
		ValueType* Find(const K& key)
		{
			Iterator it = FindIt(key);
			return it != end() ? const_cast<ValueType*>(&it->second) : nullptr;
		}

		template<typename K>
		const ValueType* Find(const K& key) const
		{
			ConstIterator it = FindIt(key);
			return it != end() ? &it->second : nullptr;
		}

		template<typename K>
		ValueType& FindRef(const K& key)
		{
			Iterator it = FindIt(key);
			assert(it != end() && "Key not found, can't dereference its value");
			return it->second;
		}

		template<typename K>
		const ValueType& FindRef(const K& key) const
		{
			ConstIterator it = FindIt(key);
			assert(it != end() && "Key not found, can't dereference its value");
			return it->second;
		}

		template<typename K>
		bool Contains(const K& key) const
		{
			return FindIt(key) != map.end();
		}

		/**
		 * Delete all items that match another provided item
		 * @return number of deleted items
		 */
		i32 Remove(const KeyType& key)
		{
			return RemoveIt(FindIt(key));
		}

		i32 RemoveIt(Iterator it)
		{
			if (it != end())
			{
				const i32 lastSize = Size();
				map.erase(it);
				return lastSize - Size();
			}
			return 0;
		}

		// Empty the map
		void Clear()
		{
			map.clear();
		}

		void Reserve(u32 size)
		{
			map.reserve(size);
		}

		void Rehash(u32 size)
		{
			map.rehash(size);
		}

		i32 Size() const
		{
			return i32(map.size());
		}

		bool IsValidIndex(i32 index) const
		{
			return index >= 0 && index < Size();
		}


		/** OPERATORS */
	public:
		/**
		 * Array bracket operator. Returns reference to value at given key.
		 *
		 * @returns Reference to indexed element.
		 */
		ValueType& operator[](const KeyType& key)
		{
			return map[key];
		}
		const ValueType& operator[](const KeyType& key) const
		{
			auto* mutThis = const_cast<TMap*>(this);
			return mutThis->map[key];
		}
		ValueType& operator[](KeyType&& key)
		{
			return map[Move(key)];
		}
		const ValueType& operator[](KeyType&& key) const
		{
			auto* mutThis = const_cast<TMap*>(this);
			return mutThis->map[Move(key)];
		}

		// Iterator functions
		Iterator begin()
		{
			return map.begin();
		};
		ConstIterator begin() const
		{
			return map.begin();
		};
		ConstIterator cbegin() const
		{
			return map.cbegin();
		};

		Iterator end()
		{
			return map.end();
		};
		ConstIterator end() const
		{
			return map.end();
		};
		ConstIterator cend() const
		{
			return map.cend();
		};


		/** INTERNAL */
	private:
		void CopyFrom(const TMap<KeyType, ValueType>& other)
		{
			map = other.map;
		}

		void MoveFrom(TMap<KeyType, ValueType>&& other)
		{
			map = Move(other.map);
		}
	};
}    // namespace p::core

namespace p
{
	using namespace p::core;
}