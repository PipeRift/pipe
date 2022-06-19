
// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/ECS/Id.h"
#include "Pipe/ECS/SparseSet.h"

#include <Pipe/Core/BitArray.h>
#include <Pipe/Core/Broadcast.h>
#include <Pipe/Core/Map.h>
#include <Pipe/Core/Set.h>
#include <Pipe/Core/Span.h>
#include <Pipe/Core/TypeTraits.h>
#include <Pipe/Memory/UniquePtr.h>
#include <Pipe/Memory/STLAllocator.h>


namespace p::ecs
{
	struct Context;


	template<typename Allocator = Memory::TDefaultAllocator<Id>>
	struct TPoolSet : public BasicSparseSet<STLAllocator<Id, Allocator>>
	{
		using Super = BasicSparseSet<STLAllocator<Id, Allocator>>;

	public:
		using Super::Super;

		void PopSwap(Id id)
		{
			Super::SwapAndPop(id, nullptr);
		}

		void Pop(Id id)
		{
			Super::InPlacePop(id, nullptr);
		}
	};


	template<typename T, typename Allocator>
	struct TPoolData
	{
		static constexpr sizet chunkSize = 1024;

		using AllocatorTraits = std::allocator_traits<STLAllocator<T, Allocator>>;
		static_assert(AllocatorTraits::propagate_on_container_move_assignment::value);
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

		STLAllocator<T, Allocator> allocator;
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
				chunks.Add(AllocatorTraits::allocate(allocator, chunkSize));
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
				AllocatorTraits::deallocate(allocator, chunks[pos], chunkSize);
			}
			chunks.RemoveLast(chunks.Size() - usedChunks);
		}

		// Release chunks to an specific size. No destructors are called
		void Reset()
		{
			for (Chunk chunk : chunks)
			{
				AllocatorTraits::deallocate(allocator, chunk, chunkSize);
			}
			chunks.Empty();
		}

		T* Push(sizet index, T&& value)
		{
			CheckMsg(index < (chunks.Size() * chunkSize), "Out of bounds index");
			++size;
			T* instance = chunks[GetChunk(index)] + GetOffset(index);
			if constexpr (std::is_aggregate_v<T>)
			{
				AllocatorTraits::construct(allocator, instance, Move(value));
			}
			else
			{
				AllocatorTraits::construct(allocator, instance, Move(value));
			}
			return instance;
		}

		T* Push(sizet index, const T& value)
		{
			CheckMsg(index < (chunks.Size() * chunkSize), "Out of bounds index");
			++size;
			T* instance = chunks[GetChunk(index)] + GetOffset(index);
			if constexpr (std::is_aggregate_v<T>)
			{
				AllocatorTraits::construct(allocator, instance, value);
			}
			else
			{
				AllocatorTraits::construct(allocator, instance, value);
			}
			return instance;
		}

		void PopSwap(sizet index, sizet last)
		{
			T& item     = chunks[GetChunk(index)][GetOffset(index)];
			T& lastItem = chunks[GetChunk(last)][GetOffset(last)];
			Swap(item, lastItem);
			AllocatorTraits::destroy(allocator, &lastItem);
			--size;
		}
		void Pop(sizet index)
		{
			T& item = chunks[GetChunk(index)][GetOffset(index)];
			AllocatorTraits::destroy(allocator, &item);
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
		void Reset() {}
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


	struct Pool
	{
		using Index = IdTraits<Id>::Index;

		using Iterator        = TPoolSet<>::iterator;
		using ReverseIterator = TPoolSet<>::reverse_iterator;


	protected:
		TPoolSet<> set;
		DeletionPolicy deletionPolicy;

		Context* context = nullptr;
		TBroadcast<Context&, TSpan<const Id>> onAdd;
		TBroadcast<Context&, TSpan<const Id>> onRemove;


		Pool(Context& ast, DeletionPolicy inDeletionPolicy)
		    : context{&ast}
		    , set{DeletionPolicy(u8(inDeletionPolicy))}
		    , deletionPolicy{inDeletionPolicy}
		{}

		void OnAdded(TSpan<const Id> ids)
		{
			onAdd.Broadcast(*context, ids);
		}

		void OnRemoved(TSpan<const Id> ids)
		{
			onRemove.Broadcast(*context, ids);
		}

	public:
		virtual ~Pool() {}

		bool Has(Id id) const
		{
			return set.Contains(id);
		}

		// Returns the data pointer of a component if contianed
		virtual void* TryGetVoid(Id id) = 0;

		virtual bool Remove(Id id)                     = 0;
		virtual void RemoveUnsafe(Id id)               = 0;
		virtual i32 Remove(TSpan<const Id> ids)        = 0;
		virtual void RemoveUnsafe(TSpan<const Id> ids) = 0;

		virtual void SetOwnerContext(Context& destination)
		{
			context = &destination;
		}
		virtual TUniquePtr<Pool> Clone() = 0;

		Context& GetContext() const
		{
			return *context;
		}

		Iterator Find(const Id id) const
		{
			return set.Find(id);
		}

		sizet Size() const
		{
			return set.Size();
		}

		Iterator begin() const
		{
			return set.begin();
		}

		Iterator end() const
		{
			return set.end();
		}

		ReverseIterator rbegin() const
		{
			return set.rbegin();
		}

		ReverseIterator rend() const
		{
			return set.rend();
		}

		TBroadcast<Context&, TSpan<const Id>>& OnAdd()
		{
			return onAdd;
		}

		TBroadcast<Context&, TSpan<const Id>>& OnRemove()
		{
			return onRemove;
		}
	};


	template<typename T, typename Allocator = Memory::TDefaultAllocator<T>>
	struct TPool : public Pool
	{
	private:
		TPoolData<T, Allocator> data;


	public:
		TPool(Context& ast) : Pool(ast, DeletionPolicy::InPlace), data{} {}
		~TPool() override
		{
			Reset();
		}

		void Add(Id id, const T&) requires(IsEmpty<T>)
		{
			if (Has(id))
			{
				return;
			}

			set.Emplace(id);
			OnAdded({id});
		}

		T& Add(Id id, const T& v) requires(!IsEmpty<T>)
		{
			if (Has(id))
			{
				return (Get(id) = v);
			}

			Check(!Has(id));
			const auto i = set.Slot();
			data.Reserve(i + 1u);

			T& value = *data.Push(i, v);

			const auto setI = set.Emplace(id);
			if (!Ensure(i == setI)) [[unlikely]]
			{
				Log::Error("Misplaced component");
				data.Pop(i);
			}
			else
			{
				OnAdded({id});
			}
			return value;
		}
		T& Add(Id id, T&& v = {}) requires(!IsEmpty<T>)
		{
			if (Has(id))
			{
				return (Get(id) = Move(v));
			}

			const auto i = set.Slot();
			data.Reserve(i + 1u);

			T& value = *data.Push(i, Forward<T>(v));

			const auto setI = set.Emplace(id);
			if (!Ensure(i == setI)) [[unlikely]]
			{
				Log::Error("Misplaced component");
				data.Pop(i);
			}
			else
			{
				OnAdded({id});
			}
			return value;
		}

		template<typename It>
		void Add(It first, It last, const T& value = {})
		{
			const sizet numToAdd = std::distance(first, last);
			const sizet newSize  = set.Size() + numToAdd;
			set.Reserve(newSize);
			data.Reserve(newSize);

			TArray<Id> ids;
			ids.Reserve(numToAdd);
			for (It it = first; it != last; ++it)
			{
				ids.Add(*it);
			}

			for (Id id : ids)
			{
				if (Has(id))
				{
					if constexpr (!IsCopyConstructible<T>)
					{
						// Ignore reference value since we can only move
						Get(id) = Move(T{});
					}
					else if constexpr (!IsEmpty<T>)
					{
						Get(id) = value;
					}
				}
				else
				{
					data.Push(set.Size(), value);
					set.EmplaceBack(id);
				}
			}
			OnAdded(ids);
		}

		template<typename It, typename CIt>
		void Add(It first, It last, CIt from) requires(
		    IsSame<std::decay_t<typename std::iterator_traits<CIt>::value_type>, T>)
		{
			const sizet numToAdd = std::distance(first, last);
			const sizet newSize  = set.Size() + numToAdd;
			set.Reserve(newSize);
			data.Reserve(newSize);

			TArray<Id> ids;
			ids.Reserve(numToAdd);
			for (It it = first; it != last; ++it)
			{
				ids.Add(*it);
			}

			for (Id id : ids)
			{
				if (Has(id))
				{
					if constexpr (!IsCopyConstructible<T>)
					{
						// Ignore reference value since we can only move
						Get(id) = Move(T{});
					}
					else if constexpr (!IsEmpty<T>)
					{
						Get(id) = *from;
					}
				}
				else
				{
					if constexpr (!IsCopyConstructible<T>)
					{
						// Ignore reference value since we can only move
						data.Push(set.Size(), {});
					}
					else
					{
						data.Push(set.Size(), *from);
					}
					set.EmplaceBack(id);
				}
				++from;
			}
			OnAdded(ids);
		}

		T& GetOrAdd(const Id id) requires(!IsEmpty<T>)
		{
			return Has(id) ? Get(id) : Add(id);
		}

		bool Remove(Id id) override
		{
			if (Has(id))
			{
				RemoveUnsafe(id);
				return true;
			}
			return false;
		}

		void RemoveUnsafe(Id id) override
		{
			Check(Has(id));
			OnRemoved({id});
			if (deletionPolicy == DeletionPolicy::InPlace)
				Pop(id);
			else
				PopSwap(id);
		}

		i32 Remove(TSpan<const Id> ids) override
		{
			OnRemoved(ids);
			i32 removed = 0;
			if (deletionPolicy == DeletionPolicy::InPlace)
			{
				for (Id id : ids)
				{
					if (Has(id))
					{
						Pop(id);
						++removed;
					}
				}
			}
			else
			{
				for (Id id : ids)
				{
					if (Has(id))
					{
						PopSwap(id);
						++removed;
					}
				}
			}
			return removed;
		}

		void RemoveUnsafe(TSpan<const Id> ids) override
		{
			OnRemoved(ids);
			if (deletionPolicy == DeletionPolicy::InPlace)
			{
				for (Id id : ids)
				{
					Check(Has(id));
					Pop(id);
				}
			}
			else
			{
				for (Id id : ids)
				{
					Check(Has(id));
					PopSwap(id);
				}
			}
		}

		T& Get(Id id) requires(!IsEmpty<T>)
		{
			Check(Has(id));
			return *data.Get(set.Index(id));
		}

		const T& Get(Id id) const requires(!IsEmpty<T>)
		{
			Check(Has(id));
			return *data.Get(set.Index(id));
		}

		T* TryGet(Id id)
		{
			return Has(id) ? data.Get(set.Index(id)) : nullptr;
		}

		const T* TryGet(Id id) const
		{
			return Has(id) ? data.Get(set.Index(id)) : nullptr;
		}

		void* TryGetVoid(Id id) override
		{
			return TryGet(id);
		}

		TUniquePtr<Pool> Clone() override
		{
			auto newPool = MakeUnique<TPool<T>>(*context);
			if constexpr (IsEmpty<T>)
			{
				newPool->Add(set.begin(), set.end(), {});
			}
			else
			{
				newPool->Add(set.begin(), set.end(), data.begin());
			}
			return Move(newPool);
		}

		void Reserve(sizet size)
		{
			set.Reserve(size);
			if (size > set.Size())
			{
				data.Reserve(size);
			}
		}

		void Shrink()
		{
			set.ShrinkToFit();
			data.Release(set.Size());
		}

		void Reset()
		{
			set.Clear();
			data.Reset();
		}

	private:
		void PopSwap(Id id)
		{
			data.PopSwap(set.Index(id), set.Size() - 1u);
			set.PopSwap(id);
		}

		void Pop(Id id)
		{
			data.Pop(set.Index(id));
			set.Pop(id);
		}
	};


	struct PoolInstance
	{
		TypeId componentId{};
		TUniquePtr<Pool> pool;


		PoolInstance(TypeId componentId, TUniquePtr<Pool>&& pool)
		    : componentId{componentId}, pool{Move(pool)}
		{}
		PoolInstance(PoolInstance&& other) noexcept
		{
			componentId       = other.componentId;
			other.componentId = TypeId::None();
			pool              = Move(other.pool);
		}
		explicit PoolInstance(const PoolInstance& other)
		{
			componentId = other.componentId;
			if (other.pool)
			{
				pool = other.pool->Clone();
			}
		}
		PoolInstance& operator=(PoolInstance&& other)
		{
			componentId       = other.componentId;
			other.componentId = TypeId::None();
			pool              = Move(other.pool);
			return *this;
		}
		PoolInstance& operator=(const PoolInstance& other)
		{
			componentId = other.componentId;
			pool.Reset();
			if (other.pool)
			{
				pool = other.pool->Clone();
			}
			return *this;
		}

		TypeId GetId() const
		{
			return componentId;
		}

		Pool* GetPool() const
		{
			return pool.Get();
		}

		bool operator<(const PoolInstance& other) const
		{
			return componentId.GetId() < other.componentId.GetId();
		}
	};


	i32 GetSmallestPool(TSpan<const Pool*> pools);
}    // namespace p::ecs