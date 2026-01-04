// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Core/Hash.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Extern/sparse_set.h"
#include "PipeMemory.h"
#include "PipePlatform.h"

#include <cassert>
#include <type_traits>


namespace p
{
	struct SetHash
	{
		template<typename T>
		sizet operator()(const T& v) const
		{
			return GetHash(v);
		}
	};

	template<typename Type>
	class TSet
	{
		// static_assert(std::is_nothrow_move_constructible<Value>::value ||
		//                  std::is_copy_constructible<Value>::value,
		//    "Value type must be nothrow move constructible and/or copy constructible.");

	public:
		template<typename OtherType>
		friend class TSet;

		using KeyEqual    = std::equal_to<>;
		using Allocator   = STLAllocator<Type>;
		using HashSetType = tsl::sparse_set<Type, SetHash, KeyEqual, Allocator>;

		using Iterator      = typename HashSetType::iterator;
		using ConstIterator = typename HashSetType::const_iterator;


	private:
		HashSetType set;


	public:
		TSet(Arena& arena = GetCurrentArena()) : set{Allocator{arena}} {}
		TSet(u32 defaultSize, Arena& arena = GetCurrentArena()) : set(defaultSize, Allocator{arena})
		{}
		TSet(const Type& item, Arena& arena = GetCurrentArena()) : TSet{arena}
		{
			Insert(item);
		}
		TSet(std::initializer_list<Type> initList, Arena& arena = GetCurrentArena())
		    : set{initList.begin(), initList.end(), 0, Allocator{arena}}
		{}

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

		Type& InsertRef(Type&& value)
		{
			return const_cast<Type&>(*Insert(Move(value)));
		}

		Type& InsertRef(const Type& value)
		{
			return const_cast<Type&>(*Insert(value));
		}

		Iterator InsertOrAssign(Type&& value)
		{
			Iterator it = set.find(value);
			if (it != end())
			{
				const_cast<Type&>(*it) = Move(value);
				return it;
			}
			return set.insert(Move(value)).first;
		}

		Iterator InsertOrAssign(const Type& value)
		{
			Iterator it = set.find(value);
			if (it != end())
			{
				const_cast<Type&>(*it) = value;
				return it;
			}
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

		template<typename Key = Type>
		bool Contains(const Key& key) const
		{
			return FindIt(key) != set.end();
		}
		template<typename Key = Type>
		bool Contains(const Key& key, sizet hash) const
		{
			return FindIt(key, hash) != set.end();
		}

		/**
		 * Delete all items that match another provided item
		 * @return number of deleted items
		 */
		template<typename Key = Type>
		i32 Remove(const Key& value)
		{
			return RemoveIt(FindIt(value));
		}
		template<typename Key = Type>
		i32 Remove(const Key& value, sizet hash)
		{
			return RemoveIt(FindIt(value, hash));
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
		void Clear()
		{
			set.clear();
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

		bool IsEmpty() const
		{
			return Size() == 0;
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
}    // namespace p
