
// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/BitArray.h"
#include "Pipe/Core/Broadcast.h"
#include "Pipe/Core/Span.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/ECS/Id.h"
#include "Pipe/ECS/PoolData.h"
#include "Pipe/ECS/PoolSet.h"
#include "Pipe/Memory/UniquePtr.h"


namespace p::ecs
{
	struct Context;

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


	template<typename T, typename Allocator = ArenaAllocator>
	struct TPool : public Pool
	{
		using AllocatorType = Allocator;

	private:
		TPoolData<T, AllocatorType> data;


	public:
		TPool(Context& ast) : Pool(ast, DeletionPolicy::InPlace), data{} {}
		~TPool() override
		{
			ShrinkToSize(0);
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
					set.TryEmplace(id, true);
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
					set.TryEmplace(id, true);
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
			set.Shrink();
			data.Release(set.Size());
		}

		void ShrinkToSize(i32 size)
		{
			data.ShrinkToSize(size, set);
		}

		void Reset()
		{
			ShrinkToSize(0);
			set.Clear();
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
