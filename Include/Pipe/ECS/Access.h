// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/TypeList.h"
#include "Pipe/ECS/Context.h"


namespace p
{
	enum class AccessMode : u8
	{
		Read,
		Write
	};

	struct TypeAccess
	{
		TypeId typeId   = TypeId::None();
		AccessMode mode = AccessMode::Read;

		constexpr TypeAccess() = default;
		constexpr TypeAccess(TypeId typeId, AccessMode mode) : typeId{typeId}, mode{mode} {}
	};

	template<typename T, AccessMode inMode>
	struct TTypeAccess : TypeAccess
	{
		using Type = Mut<T>;

		constexpr TTypeAccess() : TypeAccess(GetTypeId<T>(), inMode) {}
	};

	template<typename T>
	struct TRead : public TTypeAccess<T, AccessMode::Read>
	{};

	template<typename T>
	struct TWrite : public TTypeAccess<T, AccessMode::Write>
	{};

	template<typename T>
	struct TTypeAccessInfo
	{
		using Type                       = Mut<T>;
		static constexpr AccessMode mode = AccessMode::Read;
	};
	template<typename T>
		requires Derived<T, TypeAccess>
	struct TTypeAccessInfo<T>
	{
		using Type                       = typename T::Type;
		static constexpr AccessMode mode = T().mode;
	};
	template<typename T>
	using AsComponent = typename TTypeAccessInfo<T>::Type;


	template<typename... T>
	struct TAccess
	{
		template<typename... K>
		friend struct TAccess;

		using Components    = TTypeList<T...>;
		using RawComponents = TTypeList<AsComponent<T>...>;

	private:
		ecs::Context& context;
		TTuple<ecs::TPool<AsComponent<T>>*...> pools;


	public:
		TAccess(ecs::Context& context)
		    : context{context}, pools{&context.AssurePool<AsComponent<T>>()...}
		{}
		TAccess(const TAccess& other) : context{other.context}, pools{other.pools} {}

		// Construct a child access (super-set) from another access
		template<typename... T2>
		TAccess(const TAccess<T2...>& other) : context{other.context}
		{
			using Other = TAccess<T2...>;

			constexpr bool validConstants = (Other::template HasType<T>() && ...);
			constexpr bool validMutables =
			    ((Other::template IsWritable<T>() || TTypeAccessInfo<T>::mode != AccessMode::Write)
			        && ...);
			static_assert(validConstants, "Parent access lacks dependencies from this access.");
			static_assert(
			    validMutables, "Parent access lacks *mutable* dependencies from this access.");

			if constexpr (validConstants && validMutables)
			{
				pools = {std::get<ecs::TPool<AsComponent<T>>*>(other.pools)...};
			}
		}

		template<typename C>
		ecs::TPool<Mut<C>>* GetPool() const requires(IsMutable<C>)
		{
			static_assert(IsWritable<C>(), "Can't modify components of this type");
			if constexpr (IsWritable<C>())    // Prevent missleading errors if condition fails
			{
				return std::get<ecs::TPool<Mut<C>>*>(pools);
			}
			return nullptr;
		}

		template<typename C>
		const ecs::TPool<Mut<C>>* GetPool() const requires(IsConst<C>)
		{
			static_assert(IsReadable<C>(), "Can't read components of this type");
			if constexpr (IsReadable<C>())    // Prevent missleading errors if condition fails
			{
				return std::get<ecs::TPool<Mut<C>>*>(pools);
			}
			return nullptr;
		}

		template<typename C>
		ecs::TPool<Mut<C>>& AssurePool() const requires(IsMutable<C>)
		{
			return *GetPool<C>();
		}

		template<typename C>
		const ecs::TPool<Mut<C>>& AssurePool() const requires(IsConst<C>)
		{
			return *GetPool<C>();
		}

		bool IsValid(ecs::Id id) const
		{
			return context.IsValid(id);
		}

		template<typename... C>
		bool Has(ecs::Id id) const requires(sizeof...(C) >= 1)
		{
			return (GetPool<const C>()->Has(id) && ...);
		}

		template<typename C>
		decltype(auto) Add(ecs::Id id, C&& value = {}) const requires(IsSame<C, Mut<C>>)
		{
			return GetPool<C>()->Add(id, Forward<C>(value));
		}
		template<typename C>
		decltype(auto) Add(ecs::Id id, const C& value) const requires(IsSame<C, Mut<C>>)
		{
			return GetPool<C>()->Add(id, value);
		}
		template<typename C, typename... Args>
		decltype(auto) Add(ecs::Id id, Args&&... args)
		{
			return GetPool<C>()->Add(id, Forward<Args>(args)...);
		}

		// Add component to an entities (if they dont have it already)
		template<typename... C>
		void Add(ecs::Id id) const requires((IsSame<C, Mut<C>> && ...) && sizeof...(C) > 1)
		{
			(Add<C>(id), ...);
		}

		// Add component to many entities (if they dont have it already)
		template<typename C>
		decltype(auto) AddN(TSpan<const ecs::Id> ids, const C& value = {}) const
		{
			return GetPool<C>()->Add(ids.begin(), ids.end(), value);
		}

		// Add components to many entities (if they dont have it already)
		template<typename... C>
		void AddN(TSpan<const ecs::Id> ids) const
		    requires((IsSame<C, Mut<C>> && ...) && sizeof...(C) > 1)
		{
			(AddN<C>(ids), ...);
		}


		template<typename C>
		void Remove(const ecs::Id id) const requires(IsSame<C, Mut<C>>)
		{
			if (auto* pool = GetPool<C>())
			{
				pool->Remove(id);
			}
		}
		template<typename... C>
		void Remove(const ecs::Id id) const requires(sizeof...(C) > 1)
		{
			(Remove<C>(id), ...);
		}
		template<typename C>
		void Remove(TSpan<const ecs::Id> ids) const requires(IsSame<C, Mut<C>>)
		{
			if (auto* pool = GetPool<C>())
			{
				pool->Remove(ids);
			}
		}
		template<typename... C>
		void Remove(TSpan<const ecs::Id> ids) const requires(sizeof...(C) > 1)
		{
			(Remove<C>(ids), ...);
		}

		template<typename C>
		C& Get(ecs::Id id) const
		{
			return GetPool<C>()->Get(id);
		}

		template<typename C>
		C* TryGet(ecs::Id id) const
		{
			return GetPool<C>()->TryGet(id);
		}

		template<typename C>
		C& GetOrAdd(ecs::Id id) const requires(IsMutable<C>)
		{
			return GetPool<C>()->GetOrAdd(id);
		}

		i32 Size() const
		{
			static_assert(sizeof...(T) == 1, "Can only get the size of single component accesses");
			return GetPool<T...>()->Size();
		}

		template<typename C>
		i32 Size() const
		{
			return GetPool<const C>()->Size();
		}

		ecs::Context& GetContext() const
		{
			return context;
		}


		template<typename C>
		static constexpr bool HasType()
		{
			return ListContains<RawComponents, AsComponent<C>>();
		}

		template<typename C>
		static constexpr bool IsReadable()
		{
			return HasType<C>();
		}

		template<typename C>
		static constexpr bool IsWritable()
		{
			return IsMutable<C> && ListContains<Components, TWrite<AsComponent<C>>>();
		}
	};

	template<typename... T>
	using TAccessRef = const TAccess<T...>&;


	struct Access
	{
	protected:

		ecs::Context& ast;
		TArray<TypeAccess> types;
		TArray<ecs::BasePool*> pools;


	public:
		Access(ecs::Context& ast, const TArray<TypeId>& types) : ast{ast} {}

		template<typename... T>
		Access(TAccessRef<T...> access) : ast{access.ast}
		{}

		template<typename C>
		ecs::TPool<Mut<C>>* GetPool() const requires(IsMutable<C>)
		{
			return nullptr;
		}

		template<typename C>
		const ecs::TPool<Mut<C>>* GetPool() const requires(IsConst<C>)
		{
			return nullptr;
		}

	private:

		i32 GetPoolIndex() const
		{
			return 0;
		}
	};
}    // namespace p
