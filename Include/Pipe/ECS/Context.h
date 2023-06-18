// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Tag.h"
#include "Pipe/ECS/Pool.h"
#include "Pipe/Memory/UniquePtr.h"
#include "Pipe/PipeECS.h"


namespace p::ecs
{
	struct PIPE_API SortLessStatics
	{
		bool operator()(const OwnPtr& a, const OwnPtr& b) const
		{
			return a.GetId() < b.GetId();
		}

		bool operator()(TypeId a, const OwnPtr& b) const
		{
			return a < b.GetId();
		}

		bool operator()(const OwnPtr& a, TypeId b) const
		{
			return a.GetId() < b;
		}
	};

	struct PIPE_API Context
	{
	private:
		IdRegistry idRegistry;
		mutable TArray<PoolInstance> pools;
		TArray<OwnPtr> statics;


	public:
		Context();
		~Context()
		{
			Reset();
		}
		explicit Context(const Context& other) noexcept;
		explicit Context(Context&& other) noexcept;
		Context& operator=(const Context& other) noexcept;
		Context& operator=(Context&& other) noexcept;

#pragma region Entities
		Id Create();
		void Create(Id id);
		void Create(TSpan<Id> ids);
		void Destroy(Id id);
		void Destroy(TSpan<const Id> ids);

		// Reflection helpers
		void* AddDefaulted(TypeId typeId, Id id);
		void Remove(TypeId typeId, Id id);

		// Adds Component to an entity (if the entity doesnt have it already)
		template<typename C>
		decltype(auto) Add(Id id, C&& value = {}) const requires(IsSame<C, Mut<C>>)
		{
			Check(IsValid(id));
			return AssurePool<C>().Add(id, Forward<C>(value));
		}
		template<typename C>
		decltype(auto) Add(Id id, const C& value) const requires(IsSame<C, Mut<C>>)
		{
			Check(IsValid(id));
			return AssurePool<C>().Add(id, value);
		}
		// Adds Component to an entity (if the entity doesnt have it already)
		template<typename... Component>
		void Add(Id id) requires(sizeof...(Component) > 1)
		{
			Check(IsValid(id));
			(Add<Component>(id), ...);
		}

		// Add Component to many entities (if they dont have it already)
		template<typename Component>
		decltype(auto) AddN(TSpan<const Id> ids, const Component& value = {})
		{
			return AssurePool<Component>().Add(ids.begin(), ids.end(), value);
		}

		// Add Components to many entities (if they don't have it already)
		template<typename... Component>
		void AddN(TSpan<const Id> ids) requires(sizeof...(Component) > 1)
		{
			(Add<Component>(ids), ...);
		}

		template<typename Component>
		void AddN(TSpan<const Id> ids, const TSpan<const Component>& values)
		{
			Check(ids.Size() == values.Size());
			AssurePool<Component>().Add(ids.begin(), ids.end(), values.begin());
		}


		template<typename Component>
		void Remove(const Id id)
		{
			if (auto* pool = GetPool<Component>())
			{
				pool->Remove(id);
			}
		}
		template<typename... Component>
		void Remove(const Id id) requires(sizeof...(Component) > 1)
		{
			(Remove<Component>(id), ...);
		}
		template<typename Component>
		void Remove(TSpan<const Id> ids)
		{
			if (auto* pool = GetPool<Component>())
			{
				pool->Remove(ids);
			}
		}
		template<typename... Component>
		void Remove(TSpan<const Id> ids) requires(sizeof...(Component) > 1)
		{
			(Remove<Component>(ids), ...);
		}

		template<typename Component>
		Component& Get(const Id id) const
		{
			Check(IsValid(id));
			auto* const pool = GetPool<Component>();
			Check(pool);
			return pool->Get(id);
		}
		template<typename... Component>
		TTuple<Component&...> Get(const Id id) const requires(sizeof...(Component) > 1)
		{
			Check(IsValid(id));
			return std::forward_as_tuple(Get<Component>(id)...);
		}
		template<typename Component>
		Component* TryGet(const Id id) const
		{
			auto* const pool = GetPool<Component>();
			return pool ? pool->TryGet(id) : nullptr;
		}
		template<typename... Component>
		TTuple<Component*...> TryGet(const Id id) const requires(sizeof...(Component) > 1)
		{
			Check(IsValid(id));
			return std::forward_as_tuple(TryGet<Component>(id)...);
		}

		template<typename Component>
		Component& GetOrAdd(Id id)
		{
			Check(IsValid(id));
			return AssurePool<Component>().GetOrAdd(id);
		}

		template<typename Component>
		bool Has(Id id) const
		{
			const auto* pool = GetPool<Component>();
			return pool && pool->Has(id);
		}

		bool IsValid(Id id) const;
		bool IsOrphan(const Id id) const;

		template<typename Callback>
		void Each(Callback cb) const
		{
			idRegistry.Each(cb);
		}

		template<typename Callback>
		void EachOrphan(Callback cb) const
		{
			Each([this, &cb](const Id id) {
				if (IsOrphan(id))
				{
					cb(id);
				}
			});
		}

		template<typename Component>
		void ClearPool()
		{
			if (auto* pool = GetPool<Component>())
			{
				pool->Clear();
			}
		}

		template<typename... Component>
		void ClearPool() requires(sizeof...(Component) > 1)
		{
			(ClearPool<Component>(), ...);
		}

		void Reset(bool keepStatics = false);

		template<typename Component>
		TBroadcast<Context&, TSpan<const Id>>& OnAdd()
		{
			return AssurePool<Component>().OnAdd();
		}

		template<typename Component>
		TBroadcast<Context&, TSpan<const Id>>& OnRemove()
		{
			return AssurePool<Component>().OnRemove();
		}

		// Finds or creates a pool
		template<typename T>
		TPool<Mut<T>>& AssurePool() const;

		BasePool* GetPool(TypeId componentId) const;

		template<typename T>
		CopyConst<TPool<Mut<T>>, T>* GetPool() const
		{
			return static_cast<CopyConst<TPool<Mut<T>>, T>*>(GetPool(GetTypeId<Mut<T>>()));
		}

		const TArray<PoolInstance>& GetPools() const
		{
			return pools;
		}
#pragma endregion Entities

#pragma region Statics
		void* TryGetStatic(TypeId typeId);
		const void* TryGetStatic(TypeId typeId) const;
		bool HasStatic(TypeId typeId) const;
		bool RemoveStatic(TypeId typeId);

		template<typename Static>
		Static& SetStatic(Static&& value = {});
		template<typename Static>
		Static& SetStatic(const Static& value);
		template<typename Static>
		Static& GetOrSetStatic(Static&& newValue = {});
		template<typename Static>
		Static& GetOrSetStatic(const Static& newValue);
		template<typename Static>
		Static& GetStatic()
		{
			return *TryGetStatic<Static>();
		}
		template<typename Static>
		const Static& GetStatic() const
		{
			return *TryGetStatic<Static>();
		}
		template<typename Static>
		Static* TryGetStatic()
		{
			return static_cast<Static*>(TryGetStatic(GetTypeId<Mut<Static>>()));
		}
		template<typename Static>
		const Static* TryGetStatic() const
		{
			return static_cast<const Static*>(TryGetStatic(GetTypeId<Mut<Static>>()));
		}
		template<typename Static>
		bool HasStatic() const
		{
			return HasStatic(GetTypeId<Mut<Static>>());
		}
		template<typename Static>
		bool RemoveStatic()
		{
			return RemoveStatic(GetTypeId<Mut<Static>>());
		}
#pragma endregion Statics

	private:
		void CopyFrom(const Context& other);
		void MoveFrom(Context&& other);

		OwnPtr& FindOrAddStaticPtr(
		    TArray<OwnPtr>& statics, const TypeId typeId, bool* bAdded = nullptr);

		template<typename T>
		PoolInstance CreatePoolInstance() const;
	};

	template<typename T>
	inline TPool<Mut<T>>& Context::AssurePool() const
	{
		constexpr TypeId componentId = GetTypeId<Mut<T>>();

		i32 index = pools.LowerBound(PoolInstance{componentId, {}});
		if (index != NO_INDEX)
		{
			if (componentId != pools[index].GetId())
			{
				pools.Insert(index, CreatePoolInstance<T>());
			}
		}
		else
		{
			index = pools.Add(CreatePoolInstance<T>());
		}

		BasePool* pool = pools[index].GetPool();
		return *static_cast<TPool<Mut<T>>*>(pool);
	}

	template<typename T>
	inline PoolInstance Context::CreatePoolInstance() const
	{
		constexpr TypeId componentId = GetTypeId<Mut<T>>();

		Context& self = const_cast<Context&>(*this);
		PoolInstance instance{componentId, MakeUnique<TPool<Mut<T>>>(self)};
		return Move(instance);
	}

	template<typename Static>
	inline Static& Context::SetStatic(Static&& value)
	{
		OwnPtr& ptr = FindOrAddStaticPtr(statics, GetTypeId<Static>());
		ptr         = MakeOwned<Static>(Forward<Static>(value));
		return *ptr.GetUnsafe<Static>();
	}

	template<typename Static>
	inline Static& Context::SetStatic(const Static& value)
	{
		OwnPtr& ptr = FindOrAddStaticPtr(statics, GetTypeId<Static>());
		ptr         = MakeOwned<Static>(value);
		return *ptr.GetUnsafe<Static>();
	}

	template<typename Static>
	inline Static& Context::GetOrSetStatic(Static&& value)
	{
		bool bAdded = false;
		OwnPtr& ptr = FindOrAddStaticPtr(statics, GetTypeId<Static>(), &bAdded);
		if (bAdded)
		{
			ptr = MakeOwned<Static>(Forward<Static>(value));
		}
		return *ptr.GetUnsafe<Static>();
	}

	template<typename Static>
	inline Static& Context::GetOrSetStatic(const Static& value)
	{
		bool bAdded = false;
		OwnPtr& ptr = FindOrAddStaticPtr(statics, GetTypeId<Static>(), &bAdded);
		if (bAdded)
		{
			ptr = MakeOwned<Static>(value);
		}
		return *ptr.GetUnsafe<Static>();
	}
}    // namespace p::ecs
