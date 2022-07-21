
// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Core/Broadcast.h"
#include "Pipe/ECS/Id.h"
#include "Pipe/Memory/Alloc.h"


namespace p::ecs
{
	template<typename PackedContainerType>
	struct PoolIterator final
	{
		using difference_type   = typename IdTraits<Id>::Difference;
		using value_type        = ecs::Id;
		using pointer           = const value_type*;
		using reference         = const value_type&;
		using iterator_category = std::random_access_iterator_tag;

		PoolIterator() = default;

		PoolIterator(const PackedContainerType& ref, const difference_type offset)
		    : packed{std::addressof(ref)}, offset{offset}
		{}

		PoolIterator& operator++()
		{
			return --offset, *this;
		}

		PoolIterator operator++(int)
		{
			PoolIterator orig = *this;
			return ++(*this), orig;
		}

		PoolIterator& operator--()
		{
			return ++offset, *this;
		}

		PoolIterator operator--(int)
		{
			PoolIterator orig = *this;
			return operator--(), orig;
		}

		PoolIterator& operator+=(const difference_type value)
		{
			offset -= value;
			return *this;
		}

		PoolIterator operator+(const difference_type value) const
		{
			PoolIterator copy = *this;
			return (copy += value);
		}

		PoolIterator& operator-=(const difference_type value)
		{
			return (*this += -value);
		}

		PoolIterator operator-(const difference_type value) const
		{
			return (*this + -value);
		}

		difference_type operator-(const PoolIterator& other) const
		{
			return other.offset - offset;
		}

		bool operator==(const PoolIterator& other) const
		{
			return other.offset == offset;
		}

		auto operator<=>(const PoolIterator& other) const
		{
			return other.offset <=> offset;
		}

		reference operator[](const difference_type value) const
		{
			return packed->Data()[Index() - value];
		}

		pointer operator->() const
		{
			return packed->Data() + Index();
		}

		reference operator*() const
		{
			return *operator->();
		}

		constexpr difference_type Index() const noexcept
		{
			return offset - 1;
		}

	private:
		const PackedContainerType* packed = nullptr;
		difference_type offset            = 0;
	};


	enum class DeletionPolicy : u8
	{
		Swap,
		InPlace
	};

	struct TPoolSet
	{
		static constexpr auto growthFactor = 1.5;
		static constexpr auto pageSize     = 4096;

		using traits_type         = IdTraits<Id>;
		using PackedContainerType = TArray<Id>;

		using Entity           = traits_type::Entity;
		using Index            = traits_type::Index;
		using Version          = traits_type::Version;
		using pointer          = Id* const;
		using iterator         = PoolIterator<PackedContainerType>;
		using reverse_iterator = std::reverse_iterator<iterator>;


	private:
		TArray<Id*> pages;
		PackedContainerType packed;
		Arena* arena = nullptr;
		Id freeList;
		DeletionPolicy mode;


	public:
		explicit TPoolSet(
		    DeletionPolicy pol = DeletionPolicy::Swap, Arena& arena = GetCurrentArena())
		    : arena{&arena}, freeList{ecs::NoId}, mode{pol}
		{}
		TPoolSet(TPoolSet&& other) noexcept
		    : pages{Move(other.pages)}
		    , packed{Move(other.packed)}
		    , arena{other.arena}
		    , freeList{Exchange(other.freeList, ecs::NoId)}
		    , mode{other.mode}
		{}
		TPoolSet& operator=(TPoolSet&& other) noexcept
		{
			ReleasePages();
			pages    = Move(other.pages);
			packed   = Move(other.packed);
			arena    = other.arena;
			freeList = Exchange(other.freeList, ecs::NoId);
			mode     = other.mode;
			return *this;
		}
		~TPoolSet()
		{
			ReleasePages();
		}

		void Reserve(const i32 capacity)
		{
			packed.Reserve(capacity);
		}

		i32 Capacity() const
		{
			return packed.Capacity();
		}

		void Shrink()
		{
			packed.Shrink();
		}

		/**
		 * @returns the extent of a sparse set.
		 *
		 * The extent of a sparse set is also the size of the internal sparse array.
		 * There is no guarantee that the internal packed array has the same size.
		 * Usually the size of the internal sparse array is equal or greater than
		 * the one of the internal packed array.
		 */
		Index Extent() const
		{
			return pages.Size() * pageSize;
		}

		/**
		 * @returns the number of elements in the pool set.
		 *
		 * The number of elements is also the size of the internal packed array.
		 * There is no guarantee that the internal sparse array has the same size.
		 * Usually the size of the internal sparse array is equal or greater than
		 * the one of the internal packed array.
		 */
		i32 Size() const
		{
			return packed.Size();
		}

		bool IsEmpty() const
		{
			return packed.IsEmpty();
		}

		/**
		 * @return direct access to the internal packed array
		 */
		pointer Data() const
		{
			return packed.Data();
		}

		// Returns the next slot available for insertion
		Index Slot() const
		{
			if (freeList == ecs::NoId)
				return packed.Size();
			else
				return static_cast<Index>(ecs::GetIndex(freeList));
		}

		/**
		 * @brief Finds an entity.
		 * @param id A valid entity identifier.
		 * @return An iterator to the given entity if it's found, past the end
		 * iterator otherwise.
		 */
		iterator Find(const Id id) const
		{
			return Contains(id) ? --(end() - Index(id)) : end();
		}

		/**
		 * @brief Checks if a sparse set contains an entity.
		 * @param id A valid entity identifier.
		 * @return True if the sparse set contains the entity, false otherwise.
		 */
		bool Contains(const Id id) const
		{
			const auto index = ecs::GetIndex(id);
			const Id* pageId = GetPageIdPtrFromId(id);
			return pageId
			    && (((~ecs::NoIndex & ecs::GetIndex(id)) ^ ecs::GetIndex(*pageId)) < ecs::NoIndex);
		}

		/**
		 * @returns the contained version for an identifier
		 */
		Version GetCurrentVersion(const Id id) const noexcept
		{
			const Id* pageId = GetPageIdPtrFromId(id);
			return pageId ? GetVersion(*pageId) : NoVersion;
		}

		/**
		 * @brief Returns the position of an entity in a sparse set.
		 *
		 * @warning
		 * Attempting to get the position of an entity that doesn't belong to the
		 * sparse set results in undefined behavior.
		 *
		 * @param id A valid entity identifier.
		 * @return The position of the entity in the sparse set.
		 */
		Index GetIndex(const Id id) const
		{
			CheckMsg(Contains(id), "Set does not contain entity");
			const auto idIdx = ecs::GetIndex(id);
			return static_cast<Index>(ecs::GetIndex(GetPageId(idIdx)));
		}

		Index Emplace(Id id)
		{
			return TryEmplace(id, false);
		}

		/**
		 * @brief Assigns one or more entities to a sparse set.
		 *
		 * @warning
		 * Attempting to assign an entity that already belongs to the sparse set
		 * results in undefined behavior.
		 *
		 * @tparam It Type of input iterator.
		 * @param first An iterator to the first element of the range of entities.
		 * @param last An iterator past the last element of the range of entities.
		 */
		template<typename It>
		void Insert(It first, It last)
		{
			Reserve(packed.Size() + std::distance(first, last));

			for (auto it = first; it != last; ++it)
			{
				TryEmplace(*it, true);
			}
		}

		/*! @brief Removes all ecs::NoIds from the packed array of a sparse set. */
		void Compact()
		{
			Index from = packed.Size();
			for (; from && ecs::GetVersion(packed[i32(from) - 1u]) == NoVersion; --from) {}

			for (auto* it = &freeList; *it != NoId && from; it = &packed[i32(ecs::GetIndex(*it))])
			{
				const Index to = ecs::GetIndex(*it);
				if (to < from)
				{
					--from;
					// MoveElement(from, to);

					auto& packedTo = packed[i32(to)];
					p::Swap(packed[from], packedTo);

					GetPageIdFromId(packedTo) = ecs::MakeId(to, ecs::GetVersion(packedTo));
					*it                       = ecs::MakeId(from, NoVersion);

					for (; from && ecs::GetVersion(packed[from - 1u]) == NoVersion; --from) {}
				}
			}

			freeList = ecs::NoId;
			packed.Resize(from);
		}

		/**
		 * @copybrief swap_at
		 *
		 * For what it's worth, this function affects both the internal sparse array
		 * and the internal packed array. Users should not care of that anyway.
		 *
		 * @warning
		 * Attempting to swap entities that don't belong to the sparse set results
		 * in undefined behavior.
		 *
		 * @param lhs A valid entity identifier.
		 * @param rhs A valid entity identifier.
		 */
		void SwapElements(const Id a, const Id b)
		{
			CheckMsg(Contains(a), "Set does not contain entity");
			CheckMsg(Contains(b), "Set does not contain entity");

			auto& aPageId = GetPageIdFromId(a);
			auto& bPageId = GetPageIdFromId(b);

			const auto aIndex = ecs::GetIndex(aPageId);
			const auto bIndex = ecs::GetIndex(bPageId);

			// basic no-leak guarantee (with invalid state) if swapping throws
			// SwapAt(aIndex, bIndex);
			Id& packedA = packed[aIndex];
			Id& packedB = packed[bIndex];
			aPageId     = MakeId(bIndex, ecs::GetVersion(packedA));
			bPageId     = MakeId(aIndex, ecs::GetVersion(packedB));

			Swap(packedA, packedB);
		}

		/**
		 * @brief Sort the first count elements according to the given comparison
		 * function.
		 *
		 * The comparison function object must return `true` if the first element
		 * is _less_ than the second one, `false` otherwise. The signature of the
		 * comparison function should be equivalent to the following:
		 *
		 * @code{.cpp}
		 * bool(const ecs::Id, const ecs::Id);
		 * @endcode
		 *
		 * Moreover, the comparison function object shall induce a
		 * _strict weak ordering_ on the values.
		 *
		 * The sort function object must offer a member function template
		 * `operator()` that accepts three arguments:
		 *
		 * * An iterator to the first element of the range to sort.
		 * * An iterator past the last element of the range to sort.
		 * * A comparison function to use to compare the elements.
		 *
		 * @tparam Compare Type of comparison function object.
		 * @tparam Sort Type of sort function object.
		 * @tparam Args Types of arguments to forward to the sort function object.
		 * @param length Number of elements to sort.
		 * @param compare A valid comparison function object.
		 */
		template<typename Compare>
		void SortN(const Index length, Compare compare)
		{
			// basic no-leak guarantee (with invalid state) if sorting throws
			CheckMsg(!(length > packed.Size()), "Length exceeds the number of elements");
			Compact();

			packed.SortRange(0, length, Move(compare));

			for (Index pos{}; pos < length; ++pos)
			{
				Index curr = pos;
				Index next = ecs::GetIndex(packed[curr]);

				while (curr != next)
				{
					const Index idx = ecs::GetIndex(packed[next]);
					const Id id     = packed[curr];

					SwapAt(next, idx);
					GetPageIdRefFromId(id) =
					    ecs::MakeId(Entity(curr), ecs::GetVersion(packed[curr]));

					curr = Exchange(next, idx);
				}
			}
		}

		/**
		 * @brief Sort all elements according to the given comparison function.
		 *
		 * @sa sort_n
		 *
		 * @tparam Compare Type of comparison function object.
		 * @tparam Sort Type of sort function object.
		 * @tparam Args Types of arguments to forward to the sort function object.
		 * @param compare A valid comparison function object.
		 */
		template<typename Compare>
		void Sort(Compare compare)
		{
			Compact();
			SortN(packed.Size(), Move(compare));
		}

		/**
		 * @brief Sort entities according to their order in another sparse set.
		 *
		 * Entities that are part of both the sparse sets are ordered internally
		 * according to the order they have in `other`. All the other entities goes
		 * to the end of the list and there are no guarantees on their order.<br/>
		 * In other terms, this function can be used to impose the same order on two
		 * sets by using one of them as a master and the other one as a slave.
		 *
		 * Iterating the sparse set with a couple of iterators returns elements in
		 * the expected order after a call to `respect`. See `begin` and `end` for
		 * more details.
		 *
		 * @param other The sparse sets that imposes the order of the entities.
		 */
		void Respect(const TPoolSet& other)
		{
			Compact();

			const iterator to = other.end();
			iterator from     = other.begin();
			for (Index index = packed.Size() - 1; index && from != to; ++from)
			{
				if (Contains(*from))
				{
					if (*from != packed[index])
					{
						// basic no-leak guarantee (with invalid state) if swapping throws
						SwapElements(packed[index], *from);
					}
					--index;
				}
			}
		}

		/**
		 * @brief Clears a sparse set.
		 */
		void Clear()
		{
			if (const auto last = end(); freeList == NoId)
			{
				Pop(begin(), last);
			}
			else
			{
				for (Id entity : *this)
				{
					if (ecs::GetVersion(entity) != ecs::GetVersion(ecs::NoId))
					{
						Pop(&entity, &entity + 1u);
					}
				}
			}

			freeList = ecs::NoId;
			packed.Clear();
		}


		template<typename It>
		void PopSwap(It first, It last)
		{
			for (; first != last; ++first)
			{
				const Id lastPacked = packed.Last();

				const Index index           = ecs::GetIndex(*first);
				GetPageIdFromId(lastPacked) = MakeId(index, ecs::GetVersion(lastPacked));
				const Id id                 = Exchange(packed[index], lastPacked);

				// unnecessary but it helps to detect nasty bugs
				CheckMsg((packed.Last() = ecs::NoId, true), "");
				// lazy self-assignment guard
				GetPageIdFromId(id) = ecs::NoId;
				packed.RemoveLast();
			}
		}

		template<typename It>
		void Pop(It first, It last)
		{
			for (; first != last; ++first)
			{
				const Index index = ecs::GetIndex(*first);
				GetPageId(index)  = ecs::NoId;
				packed[index]     = Exchange(freeList, ecs::MakeId(index, NoVersion));
			}
		}

		/**
		 * @brief Appends an entity to a sparse set.
		 *
		 * @warning
		 * Attempting to assign an entity that already belongs to the sparse set
		 * results in undefined behavior.
		 *
		 * @param id A valid entity identifier.
		 * @return The slot used for insertion.
		 */
		Index TryEmplace(const Id id, bool forceBack)
		{
			CheckMsg(!Contains(id), "Set already contains entity");
			const auto idIndex = ecs::GetIndex(id);
			auto& item         = AssurePage(GetPage(idIndex))[GetOffset(idIndex)];
			if (freeList == ecs::NoId || forceBack)
			{
				item = ecs::MakeId(static_cast<typename traits_type::Entity>(packed.Size()));
				packed.Add(id);
				return packed.Size() - 1;
			}
			else
			{
				const auto index = static_cast<Index>(ecs::GetIndex(freeList));
				item             = ecs::MakeId(static_cast<typename traits_type::Entity>(index));
				freeList         = Exchange(packed[index], id);
				return index;
			}
		}

		/**
		 * @brief Returns the entity at specified location, with bounds checking.
		 * @param pos The position for which to return the entity.
		 * @return The entity at specified location if any, a nullptr entity otherwise.
		 */
		Id At(const Index index) const
		{
			return index < packed.Size() ? packed[index] : NoId;
		}

		/**
		 * @brief Returns the entity at specified location, without bounds checking.
		 * @param pos The position for which to return the entity.
		 * @return The entity at specified location.
		 */
		Id operator[](const Index index) const
		{
			CheckMsg(packed.IsValidIndex(index), "Index is out of bounds");
			return packed[index];
		}

		Id* GetPageIdPtr(Index index) const
		{
			const i32 pageId = GetPage(index);
			if (pageId < pages.Size() && pages[pageId])
			{
				return pages[pageId] + GetOffset(index);
			}
			return nullptr;
		}

		Id& GetPageId(Index index) const
		{
			const i32 pageId = GetPage(index);
			CheckMsg(pageId < pages.Size() && pages[pageId], "Invalid element");
			return pages[pageId][GetOffset(index)];
		}

		Id* GetPageIdPtrFromId(const Id id) const
		{
			return GetPageIdPtr(ecs::GetIndex(id));
		}

		Id& GetPageIdFromId(Id id) const
		{
			return GetPageId(ecs::GetIndex(id));
		}


	private:

		static i32 GetPage(sizet index)
		{
			return static_cast<i32>(index / pageSize);
		}

		static Index GetOffset(sizet index)
		{
			return static_cast<Index>(index & (pageSize - 1));
		}


		Id* AssurePage(i32 pageIndex)
		{
			if (pageIndex >= pages.Size())
			{
				pages.Resize(pageIndex + 1, nullptr);
			}

			Id*& page = pages[pageIndex];
			if (!page)
			{
				page = p::Alloc<Id>(*arena, pageSize);
				std::uninitialized_fill(page, page + pageSize, ecs::NoId);
			}

			return page;
		}

		void ReleasePages()
		{
			for (Id* page : pages)
			{
				if (page)
				{
					std::destroy(page, page + pageSize);
					p::Free<Id>(*arena, page, pageSize);
				}
			}
		}


		// Standard functions:
	public:

		iterator begin() const
		{
			return iterator{packed, static_cast<Index>(packed.Size())};
		}
		iterator end() const
		{
			return iterator{packed, {}};
		}
		iterator cbegin() const
		{
			return begin();
		}
		iterator cend() const
		{
			return end();
		}
		reverse_iterator rbegin() const
		{
			return std::make_reverse_iterator(end());
		}
		reverse_iterator rend() const
		{
			return std::make_reverse_iterator(begin());
		}
		reverse_iterator crbegin() const
		{
			return rbegin();
		}
		reverse_iterator crend() const
		{
			return rend();
		}
	};
}    // namespace p::ecs
