
// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Core/Broadcast.h"
#include "Pipe/ECS/Id.h"


namespace p::ecs
{
	enum class DeletionPolicy : u8
	{
		Swap,
		InPlace
	};

	template<typename Allocator = ArenaAllocator>
	struct TPoolSet
	{
		static constexpr auto growthFactor = 1.5;
		static constexpr auto chunkSize    = 4096;

		using traits_type         = IdTraits<Id>;
		using PackedContainerType = TArray<Id, Allocator>;


		struct Iterator final
		{
			using difference_type   = typename traits_type::Difference;
			using value_type        = ecs::Id;
			using pointer           = const value_type*;
			using reference         = const value_type&;
			using iterator_category = std::random_access_iterator_tag;

			Iterator() = default;

			Iterator(const PackedContainerType& ref, const difference_type idx)
			    : packed{std::addressof(ref)}, index{idx}
			{}

			Iterator& operator++()
			{
				return --index, *this;
			}

			Iterator operator++(int)
			{
				iterator orig = *this;
				return ++(*this), orig;
			}

			Iterator& operator--()
			{
				return ++index, *this;
			}

			Iterator operator--(int)
			{
				Iterator orig = *this;
				return operator--(), orig;
			}

			Iterator& operator+=(const difference_type value)
			{
				index -= value;
				return *this;
			}

			Iterator operator+(const difference_type value) const
			{
				Iterator copy = *this;
				return (copy += value);
			}

			Iterator& operator-=(const difference_type value)
			{
				return (*this += -value);
			}

			Iterator operator-(const difference_type value) const
			{
				return (*this + -value);
			}

			difference_type operator-(const Iterator& other) const
			{
				return other.index - index;
			}

			[[nodiscard]] reference operator[](const difference_type value) const
			{
				const auto pos = size_type(index - value - 1u);
				return (*packed)[pos];
			}

			[[nodiscard]] bool operator==(const Iterator& other) const
			{
				return other.index == index;
			}

			[[nodiscard]] bool operator!=(const Iterator& other) const
			{
				return !(*this == other);
			}

			[[nodiscard]] bool operator<(const Iterator& other) const
			{
				return index > other.index;
			}

			[[nodiscard]] bool operator>(const Iterator& other) const
			{
				return index < other.index;
			}

			[[nodiscard]] bool operator<=(const Iterator& other) const
			{
				return !(*this > other);
			}

			[[nodiscard]] bool operator>=(const Iterator& other) const
			{
				return !(*this < other);
			}

			[[nodiscard]] pointer operator->() const
			{
				const auto pos = size_type(index - 1u);
				return std::addressof((*packed)[pos]);
			}

			[[nodiscard]] reference operator*() const
			{
				return *operator->();
			}

		private:
			const PackedContainerType* packed = nullptr;
			difference_type index             = 0;
		};


	private:
		typename Allocator::template Typed<Id> allocator;
		// Id** sparse;
		// std::size_t bucket;
		TArray<Id*, Allocator> chunks;
		PackedContainerType packed;
		Id freeList;
		DeletionPolicy mode;

		static auto GetChunk(sizet index)
		{
			return static_cast<size_type>(index / chunkSize);
		}

		static auto GetOffset(sizet index)
		{
			return static_cast<size_type>(index & (chunkSize - 1));
		}

		[[nodiscard]] Id* AssurePage(const std::size_t idx)
		{
			if (idx >= chunks.Size())
			{
				chunks.Resize(idx + 1);
			}

			Id*& chunk = chunks[idx];
			if (!chunk)
			{
				chunk = allocator.Alloc(chunkSize);
				std::uninitialized_fill(chunk, chunk + chunkSize, ecs::NoId);
			}

			return chunk;
		}

		void ReleaseChunks()
		{
			packed.Empty();
			if (!chunks.IsEmpty())
			{
				for (Id* chunk : chunks)
				{
					if (chunk)
					{
						std::destroy(chunk, chunk + chunkSize);
						allocator.Free(chunk, chunkSize);
					}
				}
				chunks.Empty();
			}
		}

	protected:
		/**
		 * @brief Swaps two entities in the internal packed array.
		 * @param lhs A valid position of an entity within storage.
		 * @param rhs A valid position of an entity within storage.
		 */
		void SwapAt([[maybe_unused]] const std::size_t lhs, [[maybe_unused]] const std::size_t rhs)
		{}

		/**
		 * @brief Moves an entity in the internal packed array.
		 * @param from A valid position of an entity within storage.
		 * @param to A valid position of an entity within storage.
		 */
		void MoveAndPop(
		    [[maybe_unused]] const std::size_t from, [[maybe_unused]] const std::size_t to)
		{}

	public:
		/**
		 * @brief Attempts to erase an entity from the internal packed array.
		 * @param id A valid entity identifier.
		 * @param ud Optional user data that are forwarded as-is to derived classes.
		 */
		void PopSwap(const ecs::Id id)
		{
			const auto idIndex = ecs::GetIndex(id);
			auto& ref          = chunks[GetChunk(idIndex)][GetOffset(idIndex)];
			CheckMsg(packed[idIndex] == id, "Invalid entity identifier");

			auto& last           = packed.Last();
			const auto lastIndex = ecs::GetIndex(last);
			Get(lastIndex)       = ref;
			packed[idIndex]      = last;
			// unnecessary but it helps to detect nasty bugs
			CheckMsg((last = ecs::NoId, true), "");
			// lazy self-assignment guard
			ref = ecs::NoId;
			packed.RemoveLast();
		}

		/**
		 * @brief Attempts to erase an entity from the internal packed array.
		 * @param id A valid entity identifier.
		 * @param ud Optional user data that are forwarded as-is to derived classes.
		 */
		void Pop(const ecs::Id id)
		{
			const auto idIndex = ecs::GetIndex(id);
			CheckMsg(packed[idIndex] == id, "Invalid entity identifier");

			packed[idIndex] = std::exchange(
			    freeList, ecs::MakeId(static_cast<typename traits_type::Entity>(idIndex)));
			// lazy self-assignment guard
			Get(idIndex) = ecs::NoId;
		}

		/*! @brief Allocator type. */
		/*! @brief Unsigned integer type. */
		using size_type = traits_type::Index;
		/*! @brief Pointer type to contained entities. */
		using pointer = Id* const;
		/*! @brief Random access iterator type. */
		using iterator = Iterator;
		/*! @brief Reverse iterator type. */
		using reverse_iterator = std::reverse_iterator<iterator>;

		/**
		 * @brief Constructs an empty container with the given policy and allocator.
		 * @param pol Type of deletion policy.
		 * @param alloc Allocator to use (possibly default-constructed).
		 */
		explicit TPoolSet(DeletionPolicy pol = DeletionPolicy::Swap)
		    : freeList{ecs::NoId}, mode{pol}
		{}

		/**
		 * @brief Move constructor.
		 * @param other The instance to move from.
		 */
		TPoolSet(TPoolSet&& other) noexcept
		    : allocator{std::move(other.allocator)}
		    , chunks{std::move(other.chunks)}
		    , packed{std::move(other.packed)}
		    , freeList{std::exchange(other.freeList, ecs::NoId)}
		    , mode{other.mode}
		{}

		/*! @brief Default destructor. */
		~TPoolSet()
		{
			ReleaseChunks();
		}

		/**
		 * @brief Move assignment operator.
		 * @param other The instance to move from.
		 * @return This sparse set.
		 */
		TPoolSet& operator=(TPoolSet&& other) noexcept
		{
			ReleaseChunks();
			allocator = std::move(other.allocator);
			chunks    = std::move(other.chunks);
			packed    = std::move(other.packed);
			freeList  = std::exchange(other.freeList, ecs::NoId);
			mode      = other.mode;
			return *this;
		}

		/**
		 * @brief Returns the deletion policy of a sparse set.
		 * @return The deletion policy of the sparse set.
		 */
		[[nodiscard]] DeletionPolicy Policy() const
		{
			return mode;
		}

		/**
		 * @brief Returns the next slot available for insertion.
		 * @return The next slot available for insertion.
		 */
		[[nodiscard]] size_type Slot() const
		{
			return freeList == ecs::NoId ? packed.Size()
			                             : static_cast<size_type>(ecs::GetIndex(freeList));
		}

		/**
		 * @brief Increases the capacity of a sparse set.
		 *
		 * If the new capacity is greater than the current capacity, new storage is
		 * allocated, otherwise the method does nothing.
		 *
		 * @param cap Desired capacity.
		 */
		void Reserve(const size_type cap)
		{
			packed.Reserve(cap);
		}

		/**
		 * @brief Returns the number of elements that a sparse set has currently
		 * allocated space for.
		 * @return Capacity of the sparse set.
		 */
		[[nodiscard]] size_type Capacity() const
		{
			return packed.MaxSize();
		}

		/*! @brief Requests the removal of unused capacity. */
		void Shrink()
		{
			packed.Shrink();
		}

		/**
		 * @brief Returns the extent of a sparse set.
		 *
		 * The extent of a sparse set is also the size of the internal sparse array.
		 * There is no guarantee that the internal packed array has the same size.
		 * Usually the size of the internal sparse array is equal or greater than
		 * the one of the internal packed array.
		 *
		 * @return Extent of the sparse set.
		 */
		[[nodiscard]] size_type Extent() const
		{
			return chunks.Size() * chunkSize;
		}

		/**
		 * @brief Returns the number of elements in a sparse set.
		 *
		 * The number of elements is also the size of the internal packed array.
		 * There is no guarantee that the internal sparse array has the same size.
		 * Usually the size of the internal sparse array is equal or greater than
		 * the one of the internal packed array.
		 *
		 * @return Number of elements.
		 */
		[[nodiscard]] size_type Size() const
		{
			return packed.Size();
		}

		/**
		 * @brief Checks whether a sparse set is empty.
		 * @return True if the sparse set is empty, false otherwise.
		 */
		[[nodiscard]] bool IsEmpty() const
		{
			return packed.IsEmpty();
		}

		/**
		 * @brief Direct access to the internal packed array.
		 * @return A pointer to the internal packed array.
		 */
		[[nodiscard]] pointer Data() const
		{
			return packed.Data();
		}

		/**
		 * @brief Returns an iterator to the beginning.
		 *
		 * The returned iterator points to the first entity of the internal packed
		 * array. If the sparse set is empty, the returned iterator will be equal to
		 * `end()`.
		 *
		 * @return An iterator to the first entity of the internal packed array.
		 */
		[[nodiscard]] iterator begin() const
		{
			return iterator{packed, static_cast<typename traits_type::Difference>(packed.Size())};
		}

		/**
		 * @brief Returns an iterator to the end.
		 *
		 * The returned iterator points to the element following the last entity in
		 * the internal packed array. Attempting to dereference the returned
		 * iterator results in undefined behavior.
		 *
		 * @return An iterator to the element following the last entity of the
		 * internal packed array.
		 */
		[[nodiscard]] iterator end() const
		{
			return iterator{packed, {}};
		}

		/**
		 * @brief Returns a reverse iterator to the beginning.
		 *
		 * The returned iterator points to the first entity of the reversed internal
		 * packed array. If the sparse set is empty, the returned iterator will be
		 * equal to `rend()`.
		 *
		 * @return An iterator to the first entity of the reversed internal packed
		 * array.
		 */
		[[nodiscard]] reverse_iterator rbegin() const
		{
			return std::make_reverse_iterator(end());
		}

		/**
		 * @brief Returns a reverse iterator to the end.
		 *
		 * The returned iterator points to the element following the last entity in
		 * the reversed internal packed array. Attempting to dereference the
		 * returned iterator results in undefined behavior.
		 *
		 * @return An iterator to the element following the last entity of the
		 * reversed internal packed array.
		 */
		[[nodiscard]] reverse_iterator rend() const
		{
			return std::make_reverse_iterator(begin());
		}

		/**
		 * @brief Finds an entity.
		 * @param id A valid entity identifier.
		 * @return An iterator to the given entity if it's found, past the end
		 * iterator otherwise.
		 */
		[[nodiscard]] iterator Find(const Id id) const
		{
			return Contains(id) ? --(end() - Index(id)) : end();
		}

		/**
		 * @brief Checks if a sparse set contains an entity.
		 * @param id A valid entity identifier.
		 * @return True if the sparse set contains the entity, false otherwise.
		 */
		[[nodiscard]] bool Contains(const Id id) const
		{
			const auto idIdx = ecs::GetIndex(id);
			//  Testing versions permits to avoid accessing the packed array
			const auto curr = GetChunk(idIdx);
			return ecs::GetVersion(id) != ecs::GetVersion(ecs::NoId)
			    && (curr < chunks.Size() && chunks[curr]
			        && chunks[curr][GetOffset(idIdx)] != ecs::NoId);
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
		[[nodiscard]] size_type Index(const Id id) const
		{
			CheckMsg(Contains(id), "Set does not contain entity");
			const auto idIdx = ecs::GetIndex(id);
			return static_cast<size_type>(ecs::GetIndex(Get(idIdx)));
		}

		/**
		 * @brief Returns the entity at specified location, with bounds checking.
		 * @param pos The position for which to return the entity.
		 * @return The entity at specified location if any, a nullptr entity otherwise.
		 */
		[[nodiscard]] Id At(const size_type pos) const
		{
			return pos < packed.Size() ? packed[pos] : NoId;
		}

		/**
		 * @brief Returns the entity at specified location, without bounds checking.
		 * @param pos The position for which to return the entity.
		 * @return The entity at specified location.
		 */
		[[nodiscard]] Id operator[](const size_type index) const
		{
			CheckMsg(packed.IsValidIndex(index), "Index is out of bounds");
			return packed[index];
		}

		const Id& Get(sizet index) const
		{
			return chunks[GetChunk(index)][GetOffset(index)];
		}
		Id& Get(sizet index)
		{
			return chunks[GetChunk(index)][GetOffset(index)];
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
		size_type TryEmplace(const Id id, bool forceBack)
		{
			CheckMsg(!Contains(id), "Set already contains entity");
			const auto idIndex = ecs::GetIndex(id);
			auto& item         = AssurePage(GetChunk(idIndex))[GetOffset(idIndex)];
			if (freeList == ecs::NoId || forceBack)
			{
				item = ecs::MakeId(static_cast<typename traits_type::Entity>(packed.Size()));
				packed.Add(id);
				return packed.Size() - 1;
			}
			else
			{
				const auto index = static_cast<size_type>(ecs::GetIndex(freeList));
				item             = ecs::MakeId(static_cast<typename traits_type::Entity>(index));
				freeList         = std::exchange(packed[index], id);
				return index;
			}
		}

		/**
		 * @brief Assigns an entity to a sparse set.
		 *
		 * @warning
		 * Attempting to assign an entity that already belongs to the sparse set
		 * results in undefined behavior.
		 *
		 * @param id A valid entity identifier.
		 * @return The slot used for insertion.
		 */
		size_type Emplace(const Id id)
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

			for (; first != last; ++first)
			{
				const auto id = *first;
				CheckMsg(!Contains(id), "Set already contains entity");
				const auto idIndex = ecs::GetIndex(id);
				AssurePage(GetChunk(idIndex))[GetOffset(idIndex)] =
				    ecs::MakeId(static_cast<typename traits_type::Entity>(packed.Size()));
				packed.Add(id);
			}
		}

		/**
		 * @brief Erases an entity from a sparse set.
		 *
		 * @warning
		 * Attempting to erase an entity that doesn't belong to the sparse set
		 * results in undefined behavior.
		 *
		 * @param id A valid entity identifier.
		 * @param ud Optional user data that are forwarded as-is to derived classes.
		 */
		void Erase(const Id id, void* ud = nullptr)
		{
			CheckMsg(Contains(id), "Set does not contain entity");
			(mode == DeletionPolicy::InPlace) ? Pop(id, ud) : PopSwap(id, ud);
		}

		/**
		 * @brief Erases entities from a set.
		 *
		 * @sa erase
		 *
		 * @tparam It Type of input iterator.
		 * @param first An iterator to the first element of the range of entities.
		 * @param last An iterator past the last element of the range of entities.
		 * @param ud Optional user data that are forwarded as-is to derived classes.
		 */
		template<typename It>
		void Erase(It first, It last, void* ud = nullptr)
		{
			for (; first != last; ++first)
			{
				Erase(*first, ud);
			}
		}

		/**
		 * @brief Removes an entity from a sparse set if it exists.
		 * @param id A valid entity identifier.
		 * @param ud Optional user data that are forwarded as-is to derived classes.
		 * @return True if the entity is actually removed, false otherwise.
		 */
		bool Remove(const Id id, void* ud = nullptr)
		{
			return Contains(id) && (Erase(id, ud), true);
		}

		/**
		 * @brief Removes entities from a sparse set if they exist.
		 * @tparam It Type of input iterator.
		 * @param first An iterator to the first element of the range of entities.
		 * @param last An iterator past the last element of the range of entities.
		 * @param ud Optional user data that are forwarded as-is to derived classes.
		 * @return The number of entities actually removed.
		 */
		template<typename It>
		size_type Remove(It first, It last, void* ud = nullptr)
		{
			size_type found{};

			for (; first != last; ++first)
			{
				found += Remove(*first, ud);
			}

			return found;
		}

		/*! @brief Removes all ecs::NoIds from the packed array of a sparse set. */
		void Compact()
		{
			size_type from = packed.Size();
			for (; from && ecs::GetVersion(packed[from - 1u]) == ecs::GetVersion(ecs::NoId); --from)
			{}

			for (auto* it = &freeList; *it != ecs::NoId && from;
			     it       = std::addressof(packed[ecs::GetIndex(*it)]))
			{
				const size_type to = ecs::GetIndex(*it);
				if (to < from)
				{
					--from;
					MoveAndPop(from, to);
					std::swap(packed[from], packed[to]);


					const auto packedToIdx = ecs::GetIndex(packed[to]);
					chunks[GetChunk(packedToIdx)][GetOffset(packedToIdx)] =
					    ecs::MakeId(static_cast<const typename traits_type::Entity>(to));
					*it = ecs::MakeId(static_cast<typename traits_type::Entity>(from));

					for (; from && ecs::GetVersion(packed[from - 1u]) == ecs::GetVersion(ecs::NoId);
					     --from)
					{}
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
		void Swap(const Id lhs, const Id rhs)
		{
			CheckMsg(Contains(lhs), "Set does not contain entity");
			CheckMsg(Contains(rhs), "Set does not contain entity");

			const auto lhsIdx = ecs::GetIndex(lhs);
			const auto rhsIdx = ecs::GetIndex(rhs);
			auto& id          = chunks[GetChunk(lhsIdx)][GetOffset(lhsIdx)];
			auto& other       = chunks[GetChunk(rhsIdx)][GetOffset(rhsIdx)];

			const auto from = static_cast<size_type>(ecs::GetIndex(id));
			const auto to   = static_cast<size_type>(ecs::GetIndex(other));

			// basic no-leak guarantee (with invalid state) if swapping throws
			SwapAt(from, to);
			std::swap(id, other);
			packed.Swap(from, to);
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
		 * @param args Arguments to forward to the sort function object, if any.
		 */
		template<typename Compare, typename... Args>
		void SortN(const size_type length, Compare compare, Args&&... args)
		{
			// basic no-leak guarantee (with invalid state) if sorting throws
			CheckMsg(!(length > packed.Size()), "Length exceeds the number of elements");
			Compact();

			std::sort(std::forward<Args>(args)..., std::make_reverse_iterator(packed + length),
			    std::make_reverse_iterator(packed), std::move(compare));

			for (size_type pos{}; pos < length; ++pos)
			{
				auto curr = pos;
				auto next = Index(packed[curr]);

				while (curr != next)
				{
					const auto idx = Index(packed[next]);
					const auto id  = packed[curr];

					SwapAt(next, idx);
					const auto idIdx = ecs::GetIndex(id);
					chunks[GetChunk(idIdx)][GetOffset(idIdx)] =
					    ecs::MakeId(static_cast<typename traits_type::Entity>(curr));
					curr = std::exchange(next, idx);
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
		 * @param algo A valid sort function object.
		 * @param args Arguments to forward to the sort function object, if any.
		 */
		template<typename Compare, typename... Args>
		void Sort(Compare compare, Args&&... args)
		{
			SortN(packed.Size(), std::move(compare), std::forward<Args>(args)...);
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

			const auto to = other.end();
			auto from     = other.begin();

			for (size_type pos = packed.Size() - 1; pos && from != to; ++from)
			{
				if (Contains(*from))
				{
					if (*from != packed[pos])
					{
						// basic no-leak guarantee (with invalid state) if swapping throws
						Swap(packed[pos], *from);
					}

					--pos;
				}
			}
		}

		/**
		 * @brief Clears a sparse set.
		 */
		void Clear()
		{
			for (auto&& entity : *this)
			{
				if (ecs::GetVersion(entity) != ecs::GetVersion(ecs::NoId))
				{
					Pop(entity);
				}
			}

			freeList = ecs::NoId;
			packed.Empty();
		}
	};
}    // namespace p::ecs
