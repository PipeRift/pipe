
// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/Span.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/ECS/BasePool.h"
#include "Pipe/ECS/Id.h"
#include "Pipe/ECS/PageBuffer.h"
#include "Pipe/Memory/UniquePtr.h"

#include <memory>


namespace p::ecs
{
	struct Context;

	template<typename T, typename Allocator = ArenaAllocator>
	struct TPool : public BasePool
	{
		using AllocatorType = Allocator;

	private:
		TPageBuffer<T, 1024, AllocatorType> data;


	public:
		TPool(Context& ast) : BasePool(ast, DeletionPolicy::InPlace), data{} {}
		TPool(const TPool& other) : BasePool(other)
		{
			if constexpr (!p::IsEmpty<T>)
			{
				data.Reserve(other.data.Capacity());
				i32 u = 0;
				for (i32 i = 0; i < other.Size(); ++i, ++u)
				{
					const Id id = other.idList[i];
					if (id != ecs::NoId)
					{
						if constexpr (IsCopyConstructible<T>)
						{
							data.Insert(u, other.data[i]);
						}
						else
						{
							data.Insert(u);
						}
					}
				}
			}
		}
		~TPool() override
		{
			Clear();
		}

		void* AddDefaulted(Id id) override
		{
			if constexpr (p::IsEmpty<T>)
			{
				Add(id);
				return nullptr;
			}
			else
			{
				return &Add(id);
			}
		}

		template<typename... Args>
		auto Add(Id id, Args&&... args) -> Select<p::IsEmpty<T>, void, T&>
		{
			if constexpr (!p::IsEmpty<T>)
			{
				if (Has(id))
				{
					return (Get(id) = T{Forward<Args>(args)...});
				}
			}

			const auto index = EmplaceId(id, false);
			if constexpr (!p::IsEmpty<T>)
			{
				data.Reserve(index + 1u);
				T* const value = data.Insert(index, Forward<Args>(args)...);
				OnAdded({id});
				return *value;
			}
			OnAdded({id});
		}

		template<typename It>
		void Add(It first, It last, const T& value = {}) requires(IsCopyConstructible<T>)
		{
			const sizet numToAdd = std::distance(first, last);

			TArray<Id> ids;
			ids.Reserve(numToAdd);
			for (It it = first; it != last; ++it)
			{
				ids.Add(*it);
			}

			const sizet newSize = Size() + numToAdd;
			idList.Reserve(newSize);
			data.Reserve(newSize);

			for (Id id : ids)
			{
				if (Has(id))
				{
					if constexpr (!p::IsEmpty<T>)
					{
						Get(id) = value;
					}
				}
				else
				{
					const auto index = EmplaceId(id, true);
					if constexpr (!p::IsEmpty<T>)
					{
						data.Insert(index, value);
					}
				}
			}
			OnAdded(ids);
		}

		template<typename It, typename CIt>
		void Add(It first, It last, CIt from) requires(
		    IsSame<std::decay_t<typename std::iterator_traits<CIt>::value_type>, T>)
		{
			const sizet numToAdd = std::distance(first, last);

			TArray<Id> ids;
			ids.Reserve(numToAdd);
			for (It it = first; it != last; ++it)
			{
				ids.Add(*it);
			}

			const sizet newSize = Size() + numToAdd;
			idList.Reserve(newSize);
			data.Reserve(newSize);

			for (Id id : ids)
			{
				if (Has(id))
				{
					if constexpr (!IsCopyConstructible<T>)
					{
						// Ignore reference value since we can only move
						Get(id) = Move(T{});
					}
					else if constexpr (!p::IsEmpty<T>)
					{
						Get(id) = *from;
					}
				}
				else
				{
					const auto index = EmplaceId(id, true);
					if constexpr (!IsCopyConstructible<T>)
					{
						// Ignore reference value since we can only move
						data.Insert(index, {});
					}
					else
					{
						data.Insert(index, *from);
					}
				}
				++from;
			}
			OnAdded(ids);
		}

		T& GetOrAdd(const Id id) requires(!p::IsEmpty<T>)
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

		T& Get(Id id) requires(!p::IsEmpty<T>)
		{
			Check(Has(id));
			const i32 index = GetIndexFromId(id);
			return data[index];
		}

		const T& Get(Id id) const requires(!p::IsEmpty<T>)
		{
			Check(Has(id));
			return data[GetIndexFromId(id)];
		}

		T* TryGet(Id id)
		{
			if (!p::IsEmpty<T> && Has(id))
			{
				return &data[GetIndexFromId(id)];
			}
			return nullptr;
		}

		const T* TryGet(Id id) const
		{
			return Has(id) ? &data[GetIndexFromId(id)] : nullptr;
		}

		void* TryGetVoid(Id id) override
		{
			return TryGet(id);
		}

		TUniquePtr<BasePool> Clone() override
		{
			return MakeUnique<TPool<T>>(*this);
		}

		void Reserve(sizet size)
		{
			idList.Reserve(size);
			if (size > Size())
			{
				data.Reserve(size);
			}
		}

		void Shrink()
		{
			idList.Shrink();
			data.Shrink(idList.Size());
		}

		void Clear() override
		{
			if constexpr (!p::IsEmpty<T>)
			{
				for (i32 i = 0; i < Size(); ++i)
				{
					if (ecs::GetVersion(idList[i]) != ecs::NoVersion)
					{
						data.RemoveAt(i);
					}
				}
				data.Clear();
			}
			ClearIds();
		}

		/*! @brief Removes all ecs::NoIds from pool */
		void Compact()
		{
			i32 from = idList.Size();
			for (; from && ecs::GetVersion(idList[from - 1]) == NoVersion; --from) {}

			for (i32 to = lastRemovedIndex; to != NO_INDEX && from;)
			{
				if (to < from)
				{
					--from;

					if constexpr (!p::IsEmpty<T>)
					{
						std::uninitialized_move_n(&data[from], 1, &data[to]);
					}

					auto& listTo = idList[i32(to)];
					p::Swap(idList[from], listTo);

					idIndices[ecs::GetIndex(listTo)] = to;
					to                               = from;

					for (; from && ecs::GetVersion(idList[from - 1]) == NoVersion; --from) {}
				}
			}

			lastRemovedIndex = NO_INDEX;
			idList.Resize(from);
		}

		void Swap(const Id a, const Id b)
		{
			CheckMsg(Has(a), "Set does not contain entity");
			CheckMsg(Has(b), "Set does not contain entity");

			i32& aListIdx = idIndices[ecs::GetIndex(a)];
			i32& bListIdx = idIndices[ecs::GetIndex(b)];

			p::Swap(idList[aListIdx], idList[bListIdx]);
			p::Swap(aListIdx, bListIdx);
			data.Swap(aListIdx, bListIdx);
		}

	private:
		void PopSwap(Id id)
		{
			if constexpr (!p::IsEmpty<T>)
			{
				const i32 lastIndex = Size() - 1u;
				data.Swap(GetIndexFromId(id), lastIndex);
				data.RemoveAt(lastIndex);
			}
			PopSwapId(id);
		}

		void Pop(Id id)
		{
			if constexpr (!p::IsEmpty<T>)
			{
				data.RemoveAt(GetIndexFromId(id));
			}
			PopId(id);
		}
	};


	struct PoolInstance
	{
		TypeId componentId{};
		TUniquePtr<BasePool> pool;


		PoolInstance(TypeId componentId, TUniquePtr<BasePool>&& pool)
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

		BasePool* GetPool() const
		{
			return pool.Get();
		}

		bool operator<(const PoolInstance& other) const
		{
			return componentId.GetId() < other.componentId.GetId();
		}
	};
}    // namespace p::ecs
