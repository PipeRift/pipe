// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Broadcast.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/Span.h"
#include "Pipe/Core/TypeList.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/ECS/PageBuffer.h"
#include "Pipe/Memory/UniquePtr.h"
#include "Pipe/Reflect/Builders/NativeTypeBuilder.h"
#include "Pipe/Reflect/Struct.h"
#include "Pipe/Serialize/Serialization.h"

namespace p
{
	struct EntityContext;


	////////////////////////////////
	// ENTITY IDS
	//

	/** An Id is an integer composed of both index and version that identifies an entity */
	enum class Id : u32
	{
	};

	/** IdTraits define properties of an Id type based on its size */
	template<typename Type>
	struct IdTraits;
	template<>
	struct IdTraits<u32>
	{
		using Entity     = u32;
		using Index      = u32;
		using Version    = u16;
		using Difference = i64;

		static constexpr Entity indexMask   = 0xfffff;
		static constexpr Entity versionMask = 0xfff;
		static constexpr sizet indexShift   = 20u;
	};
	template<>
	struct IdTraits<u64>
	{
		using Entity     = u64;
		using Index      = u32;
		using Version    = u32;
		using Difference = i64;

		static constexpr Entity indexMask   = 0xffffffff;
		static constexpr Entity versionMask = 0xffffffff;
		static constexpr sizet indexShift   = 32u;
	};
	template<>
	struct IdTraits<Id> : public IdTraits<UnderlyingType<Id>>
	{};

	// Creates an id from a combination of index and version. This does NOT create an entity.
	constexpr Id MakeId(IdTraits<Id>::Index index = IdTraits<Id>::indexMask,
	    IdTraits<Id>::Version version             = IdTraits<Id>::versionMask)
	{
		return Id{(index & IdTraits<Id>::indexMask)
		          | (IdTraits<Id>::Entity(version) << IdTraits<Id>::indexShift)};
	}

	// Extract the index from an id
	constexpr IdTraits<Id>::Index GetIdIndex(Id id)
	{
		return IdTraits<Id>::Index{IdTraits<Id>::Entity(id) & IdTraits<Id>::indexMask};
	}

	// Extract the version from an id
	constexpr IdTraits<Id>::Version GetIdVersion(Id id)
	{
		constexpr auto mask = IdTraits<Id>::versionMask << IdTraits<Id>::indexShift;
		return IdTraits<Id>::Version((IdTraits<Id>::Entity(id) & mask) >> IdTraits<Id>::indexShift);
	}

	// Invalid value of an Id
	constexpr Id NoId                           = MakeId();
	constexpr IdTraits<Id>::Version NoIdVersion = GetIdVersion(NoId);
	constexpr IdTraits<Id>::Index NoIdIndex     = GetIdIndex(NoId);

	/**
	 * @param id to check
	 * @return true if an Id contains an invalid version. Use IdRegistry::IsValid to check if it
	 * exists
	 */
	constexpr bool IsNone(Id id)
	{
		return GetIdVersion(id) == GetIdVersion(NoId);
	}


	/** IdRegistry tracks the existance and versioning of ids. Used internally by the ECS context */
	struct PIPE_API IdRegistry
	{
		using Traits  = IdTraits<Id>;
		using Index   = Traits::Index;
		using Version = Traits::Version;

	private:

		TArray<Id> entities;
		TArray<Index> available;


	public:

		IdRegistry() {}
		IdRegistry(IdRegistry&& other)                 = default;
		IdRegistry(const IdRegistry& other)            = default;
		IdRegistry& operator=(IdRegistry&& other)      = default;
		IdRegistry& operator=(const IdRegistry& other) = default;


		Id Create();
		void Create(TSpan<Id> newIds);
		bool Destroy(Id id);
		bool Destroy(TSpan<const Id> ids);
		bool IsValid(Id id) const;

		u32 Size() const
		{
			return entities.Size() - available.Size();
		}

		template<typename Callback>
		void Each(Callback cb) const;
	};


	////////////////////////////////
	// SERIALIZATION
	//

	// Mark an entity as not serialized (it will be entirely ignored by the serializer)
	struct CNotSerialized : public Struct
	{
		STRUCT(CNotSerialized, Struct)
	};

	class PIPE_API EntityReader : public Reader
	{
		EntityContext& context;

		// While serializing we create ids as Ids appear and link them.
		TArray<Id> ids;
		bool serializingMany = false;


	public:
		EntityReader(const p::Reader& parent, EntityContext& context)
		    : Reader(parent), context{context}
		{}

		void SerializeEntities(p::TArray<Id>& entities, TFunction<void(EntityReader&)> onReadPools);
		void SerializeEntity(Id& entity, TFunction<void(EntityReader&)> onReadPools);
		void SerializeSingleEntity(Id& entity, TFunction<void(EntityReader&)> onReadPools);

		template<typename T>
		void SerializePool();

		template<typename... T>
		void SerializePools()
		{
			(SerializePool<T>(), ...);
		}

		const TArray<Id>& GetIds() const;
		EntityContext& GetContext();
	};


	class PIPE_API EntityWriter : public Writer
	{
		EntityContext& context;

		// While serializing we create ids as Ids appear and link them.
		TArray<Id> ids;
		TMap<Id, i32> idToIndexes;
		bool serializingMany = false;


	public:
		EntityWriter(const Writer& parent, EntityContext& context)
		    : Writer(parent), context{context}
		{}

		void SerializeEntities(const TArray<Id>& entities,
		    TFunction<void(EntityWriter&)> onWritePools, bool includeChildren = true);
		void SerializeEntity(
		    Id entity, TFunction<void(EntityWriter&)> onWritePools, bool includeChildren = true);
		void SerializeSingleEntity(Id entity, TFunction<void(EntityWriter&)> onWritePools);

		template<typename T>
		void SerializePool();

		template<typename... T>
		void SerializePools()
		{
			(SerializePool<T>(), ...);
		}

		const TArray<Id>& GetIds() const;
		const TMap<Id, i32>& GetIdToIndexes() const;

	private:
		void RetrieveHierarchy(const TArray<Id>& roots, TArray<Id>& children);
		void RemoveIgnoredEntities(TArray<Id>& entities);
		void MapIdsToIndices();
	};

	void PIPE_API Read(p::Reader& ct, p::Id& val);
	void PIPE_API Write(p::Writer& ct, p::Id val);


	////////////////////////////////
	// POOLS
	//

	enum class DeletionPolicy : u8
	{
		Swap,
		InPlace
	};

	/** Iterates the Ids contained in a pool */
	struct PIPE_API PoolIterator final
	{
		using difference_type   = typename IdTraits<Id>::Difference;
		using value_type        = Id;
		using pointer           = const value_type*;
		using reference         = const value_type&;
		using iterator_category = std::random_access_iterator_tag;

	private:
		const TArray<Id>* idList = nullptr;
		difference_type offset   = 0;


	public:

		PoolIterator() = default;

		PoolIterator(const TArray<Id>& idList, const difference_type offset)
		    : idList{&idList}, offset{offset}
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
			return idList->Data()[Index() - value];
		}

		pointer operator->() const
		{
			return idList->Data() + Index();
		}

		reference operator*() const
		{
			return *operator->();
		}

		constexpr difference_type Index() const noexcept
		{
			return offset - 1;
		}
	};


	struct PIPE_API BasePool
	{
		using Index = IdTraits<Id>::Index;

		using Iterator        = PoolIterator;
		using ReverseIterator = std::reverse_iterator<Iterator>;


	protected:
		TPageBuffer<i32, 4096> idIndices;
		TArray<Id> idList;
		Arena* arena         = nullptr;
		i32 lastRemovedIndex = NO_INDEX;
		DeletionPolicy deletionPolicy;

		EntityContext* context = nullptr;
		TBroadcast<EntityContext&, TSpan<const Id>> onAdd;
		TBroadcast<EntityContext&, TSpan<const Id>> onRemove;


		BasePool(
		    EntityContext& ast, DeletionPolicy deletionPolicy, Arena& arena = GetCurrentArena())
		    : arena{&arena}, context{&ast}, deletionPolicy{deletionPolicy}
		{
			BindOnPageAllocated();
		}
		BasePool(const BasePool& other);
		BasePool(BasePool&& other) noexcept;
		BasePool& operator=(BasePool&& other) noexcept;

		void OnAdded(TSpan<const Id> ids)
		{
			onAdd.Broadcast(*context, ids);
		}

		void OnRemoved(TSpan<const Id> ids)
		{
			onRemove.Broadcast(*context, ids);
		}

	public:
		virtual ~BasePool() {}

		// Returns the data pointer of a component if contianed
		virtual void* TryGetVoid(Id id) = 0;

		virtual void* AddDefaulted(Id id)              = 0;
		virtual bool Remove(Id id)                     = 0;
		virtual void RemoveUnsafe(Id id)               = 0;
		virtual i32 Remove(TSpan<const Id> ids)        = 0;
		virtual void RemoveUnsafe(TSpan<const Id> ids) = 0;

		inline i32 GetIndexFromId(const Index index) const
		{
			return idIndices[index];
		}
		inline i32 GetIndexFromId(const Id id) const
		{
			return GetIndexFromId(GetIdIndex(id));
		}
		Id GetIdFromIndex(i32 index) const
		{
			return index < idList.Size() ? idList[index] : NoId;
		}

		virtual void SetOwnerContext(EntityContext& destination)
		{
			context = &destination;
		}
		virtual TUniquePtr<BasePool> Clone() = 0;

		EntityContext& GetContext() const
		{
			return *context;
		}

		bool Has(Id id) const
		{
			const i32* const index = idIndices.At(GetIdIndex(id));
			return index && *index != NO_INDEX;
		}

		Iterator Find(const Id id) const
		{
			return Has(id) ? Iterator{idList, GetIndexFromId(id)} : end();
		}

		i32 Size() const
		{
			return idList.Size();
		}

		bool IsEmpty() const
		{
			return idList.IsEmpty();
		}

		// Return pointer to internal list of ids
		Id* Data() const
		{
			return idList.Data();
		}

		TBroadcast<EntityContext&, TSpan<const Id>>& OnAdd()
		{
			return onAdd;
		}

		TBroadcast<EntityContext&, TSpan<const Id>>& OnRemove()
		{
			return onRemove;
		}

		virtual void Clear() = 0;

	protected:

		Index EmplaceId(const Id id, bool forceBack);

		void PopId(Id id);

		void PopSwapId(Id id);

		void ClearIds();


		// Standard functions:
	public:

		Iterator begin() const
		{
			return Iterator{idList, static_cast<Index>(idList.Size())};
		}
		Iterator end() const
		{
			return Iterator{idList, {}};
		}
		Iterator cbegin() const
		{
			return begin();
		}
		Iterator cend() const
		{
			return end();
		}
		ReverseIterator rbegin() const
		{
			return std::make_reverse_iterator(end());
		}
		ReverseIterator rend() const
		{
			return std::make_reverse_iterator(begin());
		}
		ReverseIterator crbegin() const
		{
			return rbegin();
		}
		ReverseIterator crend() const
		{
			return rend();
		}

	private:

		void BindOnPageAllocated();
	};


	i32 GetSmallestPool(TSpan<const BasePool*> pools);


	template<typename T, typename Allocator = ArenaAllocator>
	struct TPool : public BasePool
	{
		using AllocatorType = Allocator;

	private:
		TPageBuffer<T, 1024, AllocatorType> data;


	public:
		TPool(EntityContext& ast) : BasePool(ast, DeletionPolicy::InPlace), data{} {}
		TPool(const TPool& other) : BasePool(other)
		{
			if constexpr (!p::IsEmpty<T>)
			{
				data.Reserve(other.data.Capacity());
				i32 u = 0;
				for (i32 i = 0; i < other.Size(); ++i, ++u)
				{
					const Id id = other.idList[i];
					if (id != NoId)
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
		void Add(It first, It last, CIt from)
		    requires(IsSame<std::decay_t<typename std::iterator_traits<CIt>::value_type>, T>)
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
					if (GetIdVersion(idList[i]) != NoIdVersion)
					{
						data.RemoveAt(i);
					}
				}
				data.Clear();
			}
			ClearIds();
		}

		/*! @brief Removes all NoId from pool */
		void Compact()
		{
			i32 from = idList.Size();
			for (; from && GetIdVersion(idList[from - 1]) == NoIdVersion; --from) {}

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

					idIndices[GetIdIndex(listTo)] = to;
					to                            = from;

					for (; from && GetIdVersion(idList[from - 1]) == NoIdVersion; --from) {}
				}
			}

			lastRemovedIndex = NO_INDEX;
			idList.Resize(from);
		}

		void Swap(const Id a, const Id b)
		{
			CheckMsg(Has(a), "Set does not contain entity");
			CheckMsg(Has(b), "Set does not contain entity");

			i32& aListIdx = idIndices[GetIdIndex(a)];
			i32& bListIdx = idIndices[GetIdIndex(b)];

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


	struct PIPE_API PoolInstance
	{
		TypeId componentId{};
		TUniquePtr<BasePool> pool;

		PoolInstance(TypeId componentId, TUniquePtr<BasePool>&& pool);
		PoolInstance(PoolInstance&& other) noexcept;
		explicit PoolInstance(const PoolInstance& other);
		PoolInstance& operator=(PoolInstance&& other) noexcept;
		PoolInstance& operator=(const PoolInstance& other);
		TypeId GetId() const;
		BasePool* GetPool() const;
		bool operator<(const PoolInstance& other) const;
	};


	////////////////////////////////
	// CONTEXT
	//

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

	struct PIPE_API EntityContext
	{
	private:
		IdRegistry idRegistry;
		mutable TArray<PoolInstance> pools;
		TArray<OwnPtr> statics;


	public:
		EntityContext();
		~EntityContext()
		{
			Reset();
		}
		explicit EntityContext(const EntityContext& other) noexcept;
		explicit EntityContext(EntityContext&& other) noexcept;
		EntityContext& operator=(const EntityContext& other) noexcept;
		EntityContext& operator=(EntityContext&& other) noexcept;

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
		TBroadcast<EntityContext&, TSpan<const Id>>& OnAdd()
		{
			return AssurePool<Component>().OnAdd();
		}

		template<typename Component>
		TBroadcast<EntityContext&, TSpan<const Id>>& OnRemove()
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
		void CopyFrom(const EntityContext& other);
		void MoveFrom(EntityContext&& other);

		OwnPtr& FindOrAddStaticPtr(
		    TArray<OwnPtr>& statics, const TypeId typeId, bool* bAdded = nullptr);

		template<typename T>
		PoolInstance CreatePoolInstance() const;
	};


	////////////////////////////////
	// ACCESSES
	//

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
		EntityContext& context;
		TTuple<TPool<AsComponent<T>>*...> pools;


	public:
		TAccess(EntityContext& context)
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
				pools = {std::get<TPool<AsComponent<T>>*>(other.pools)...};
			}
		}

		template<typename C>
		TPool<Mut<C>>* GetPool() const requires(IsMutable<C>)
		{
			static_assert(IsWritable<C>(), "Can't modify components of this type");
			if constexpr (IsWritable<C>())    // Prevent missleading errors if condition fails
			{
				return std::get<TPool<Mut<C>>*>(pools);
			}
			return nullptr;
		}

		template<typename C>
		const TPool<Mut<C>>* GetPool() const requires(IsConst<C>)
		{
			static_assert(IsReadable<C>(), "Can't read components of this type");
			if constexpr (IsReadable<C>())    // Prevent missleading errors if condition fails
			{
				return std::get<TPool<Mut<C>>*>(pools);
			}
			return nullptr;
		}

		template<typename C>
		TPool<Mut<C>>& AssurePool() const requires(IsMutable<C>)
		{
			return *GetPool<C>();
		}

		template<typename C>
		const TPool<Mut<C>>& AssurePool() const requires(IsConst<C>)
		{
			return *GetPool<C>();
		}

		bool IsValid(Id id) const
		{
			return context.IsValid(id);
		}

		template<typename... C>
		bool Has(Id id) const requires(sizeof...(C) >= 1)
		{
			return (GetPool<const C>()->Has(id) && ...);
		}

		template<typename C>
		decltype(auto) Add(Id id, C&& value = {}) const requires(IsSame<C, Mut<C>>)
		{
			return GetPool<C>()->Add(id, Forward<C>(value));
		}
		template<typename C>
		decltype(auto) Add(Id id, const C& value) const requires(IsSame<C, Mut<C>>)
		{
			return GetPool<C>()->Add(id, value);
		}

		// Add component to an entities (if they dont have it already)
		template<typename... C>
		void Add(Id id) const requires((IsSame<C, Mut<C>> && ...) && sizeof...(C) > 1)
		{
			(Add<C>(id), ...);
		}

		// Add component to many entities (if they dont have it already)
		template<typename C>
		decltype(auto) AddN(TSpan<const Id> ids, const C& value = {}) const
		{
			return GetPool<C>()->Add(ids.begin(), ids.end(), value);
		}

		// Add components to many entities (if they dont have it already)
		template<typename... C>
		void AddN(TSpan<const Id> ids) const
		    requires((IsSame<C, Mut<C>> && ...) && sizeof...(C) > 1)
		{
			(AddN<C>(ids), ...);
		}


		template<typename C>
		void Remove(const Id id) const requires(IsSame<C, Mut<C>>)
		{
			if (auto* pool = GetPool<C>())
			{
				pool->Remove(id);
			}
		}
		template<typename... C>
		void Remove(const Id id) const requires(sizeof...(C) > 1)
		{
			(Remove<C>(id), ...);
		}
		template<typename C>
		void Remove(TSpan<const Id> ids) const requires(IsSame<C, Mut<C>>)
		{
			if (auto* pool = GetPool<C>())
			{
				pool->Remove(ids);
			}
		}
		template<typename... C>
		void Remove(TSpan<const Id> ids) const requires(sizeof...(C) > 1)
		{
			(Remove<C>(ids), ...);
		}

		template<typename C>
		C& Get(Id id) const
		{
			return GetPool<C>()->Get(id);
		}

		template<typename C>
		C* TryGet(Id id) const
		{
			return GetPool<C>()->TryGet(id);
		}

		template<typename C>
		C& GetOrAdd(Id id) const requires(IsMutable<C>)
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

		EntityContext& GetContext() const
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

		EntityContext& ast;
		TArray<TypeAccess> types;
		TArray<BasePool*> pools;


	public:
		Access(EntityContext& ast, const TArray<TypeId>& types) : ast{ast} {}

		template<typename... T>
		Access(TAccessRef<T...> access) : ast{access.ast}
		{}

		template<typename C>
		TPool<Mut<C>>* GetPool() const requires(IsMutable<C>)
		{
			return nullptr;
		}

		template<typename C>
		const TPool<Mut<C>>* GetPool() const requires(IsConst<C>)
		{
			return nullptr;
		}

	private:

		i32 GetPoolIndex() const
		{
			return 0;
		}
	};


	////////////////////////////////
	// DEFINITIONS
	//

	template<typename Callback>
	void IdRegistry::Each(Callback cb) const
	{
		if (available.IsEmpty())
		{
			for (i32 i = 0; i < entities.Size(); ++i)
			{
				cb(entities[i]);
			}
		}
		else
		{
			for (i32 i = 0; i < entities.Size(); ++i)
			{
				const Id id = entities[i];
				if (GetIdIndex(id) == i)
				{
					cb(id);
				}
			}
		}
	}


	template<typename T>
	inline void EntityReader::SerializePool()
	{
		if (EnterNext(GetTypeName<T>(false)))
		{
			auto& pool = GetContext().AssurePool<T>();

			if (serializingMany) [[likely]]
			{
				String key;
				BeginObject();
				for (i32 i = 0; i < ids.Size(); ++i)
				{
					const Id node = ids[i];
					key.clear();
					Strings::FormatTo(key, "{}", i);

					if (EnterNext(key))
					{
						if constexpr (!IsEmpty<T>)
						{
							T& comp = pool.GetOrAdd(node);
							Serialize(comp);
						}
						else
						{
							pool.Add(node);
						}
						Leave();
					}
				}
			}
			else
			{
				if constexpr (!IsEmpty<T>)
				{
					T& comp = pool.GetOrAdd(ids[0]);
					Serialize(comp);
				}
				else
				{
					pool.Add(ids[0]);
				}
			}
			Leave();
		}
	}

	template<typename T>
	inline void EntityWriter::SerializePool()
	{
		TArray<TPair<i32, Id>> componentIds;    // TODO: Make sure this is needed

		auto* pool = context.GetPool<const T>();
		if (pool)
		{
			componentIds.Reserve(math::Min(i32(pool->Size()), ids.Size()));
			for (i32 i = 0; i < ids.Size(); ++i)
			{
				const Id id = ids[i];
				if (pool->Has(id))
				{
					componentIds.Add({i, id});
				}
			}
		}

		if (componentIds.IsEmpty())
		{
			return;
		}

		// FIX: yyjson doesn't seem to take into account stringview length when generating text
		// Temporarely fixed by caching component name keys
		PushAddFlags(p::WriteFlags_CacheStringKeys);
		if (EnterNext(GetTypeName<T>(false)))
		{
			if (serializingMany) [[likely]]
			{
				String key;
				BeginObject();
				for (auto id : componentIds)
				{
					key.clear();
					Strings::FormatTo(key, "{}", id.first);

					if constexpr (std::is_empty_v<T>)
					{
						Next(StringView{key}, T{});
					}
					else
					{
						Next(StringView{key}, pool->Get(id.second));
					}
				}
			}
			else
			{
				if constexpr (std::is_empty_v<T>)
				{
					Serialize(T{});
				}
				else
				{
					Serialize(pool->Get(componentIds.First().second));
				}
			}
			Leave();
		}
		PopFlags();
	}


	template<typename T>
	inline TPool<Mut<T>>& EntityContext::AssurePool() const
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
	inline PoolInstance EntityContext::CreatePoolInstance() const
	{
		constexpr TypeId componentId = GetTypeId<Mut<T>>();

		auto& self = const_cast<EntityContext&>(*this);
		PoolInstance instance{componentId, MakeUnique<TPool<Mut<T>>>(self)};
		return Move(instance);
	}

	template<typename Static>
	inline Static& EntityContext::SetStatic(Static&& value)
	{
		OwnPtr& ptr = FindOrAddStaticPtr(statics, GetTypeId<Static>());
		ptr         = MakeOwned<Static>(Forward<Static>(value));
		return *ptr.GetUnsafe<Static>();
	}

	template<typename Static>
	inline Static& EntityContext::SetStatic(const Static& value)
	{
		OwnPtr& ptr = FindOrAddStaticPtr(statics, GetTypeId<Static>());
		ptr         = MakeOwned<Static>(value);
		return *ptr.GetUnsafe<Static>();
	}

	template<typename Static>
	inline Static& EntityContext::GetOrSetStatic(Static&& value)
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
	inline Static& EntityContext::GetOrSetStatic(const Static& value)
	{
		bool bAdded = false;
		OwnPtr& ptr = FindOrAddStaticPtr(statics, GetTypeId<Static>(), &bAdded);
		if (bAdded)
		{
			ptr = MakeOwned<Static>(value);
		}
		return *ptr.GetUnsafe<Static>();
	}
}    // namespace p

REFLECT_NATIVE_TYPE(p::Id);
