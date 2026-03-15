// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "Pipe/Core/Map.h"
#include "Pipe/Core/PageBuffer.h"
#include "Pipe/Core/Templates.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Memory/UniquePtr.h"
#include "PipeArrays.h"
#include "PipePlatform.h"
#include "PipeReflect.h"

#include <shared_mutex>


#ifndef P_ID_IS_64BIT
	#define P_ID_IS_64BIT 0
#endif


namespace p
{
	////////////////////////////////
	// FORWARD DECLARATIONS
	//
	struct IdContext;


	////////////////////////////////
	// DECLARATIONS
	//
	enum class RmIdFlags : u8
	{
		// Default: Deferred & keep children
		None = 0,
		// Remove entities instantly instead of waiting (deferred)
		Instant = 1 << 0,
		// Keep children entities of the ones being removed
		KeepChildren = 1 << 1
	};
	P_DEFINE_FLAG_OPERATORS(RmIdFlags)


#pragma region Ids
	/** An Id is an integer composed of both index and version that identifies an entity */
	struct P_API Id
	{
#if P_ID_IS_64BIT
		using Value                          = u64;
		using Index                          = u32;
		using Version                        = u32;
		using Difference                     = i64;
		static constexpr Index indexMask     = 0xffffffff;
		static constexpr Version versionMask = 0xffffffff;
		static constexpr sizet indexShift    = 32u;
#else
		using Value                          = u32;
		using Index                          = u32;
		using Version                        = u16;
		using Difference                     = i64;
		static constexpr Index indexMask     = 0xfffff;
		static constexpr Version versionMask = 0xfff;
		static constexpr sizet indexShift    = 20u;
#endif

		Value value;

	private:
		explicit constexpr Id(Value value) : value(value) {}

	public:

		constexpr Id() : Id(indexMask, versionMask) {}
		constexpr Id(Index index, Version version)
		    : value{(index & indexMask) | ((Value)version << indexShift)}
		{}
		constexpr Id(const Id& other) : Id(other.value) {}
		constexpr Id& operator=(const Id& other)
		{
			value = other.value;
			return *this;
		}


		constexpr bool operator==(const Id other) const
		{
			return value == other.value;
		}
		constexpr bool operator!=(const Id other) const
		{
			return value != other.value;
		}
		constexpr bool operator<(const Id other) const
		{
			return value < other.value;
		}
		constexpr bool operator<=(const Id other) const
		{
			return value <= other.value;
		}
		constexpr bool operator>(const Id other) const
		{
			return value > other.value;
		}
		constexpr bool operator>=(const Id other) const
		{
			return value >= other.value;
		}

		static constexpr Id MakeRaw(Value raw)
		{
			return Id(raw);
		}

		constexpr Index GetIndex() const
		{
			return value & indexMask;
		}
		constexpr Version GetVersion() const
		{
			constexpr auto mask = versionMask << indexShift;
			return (value & mask) >> indexShift;
		}
	};

	inline sizet GetHash(const Id id)
	{
		return GetHash(id.value);
	}

	// Creates an id from a combination of index and version. This does NOT create an entity.
	constexpr Id MakeId(Id::Index index = 0, Id::Version version = Id::versionMask)
	{
		return Id(index, version);
	}


	// Invalid value of an Id
	constexpr Id NoId                 = MakeId();
	constexpr Id::Version NoIdVersion = Id::versionMask;

	/**
	 * @param id to check
	 * @return true if an Id contains an invalid version. Use IdRegistry::IsValid to check if it
	 * exists
	 */
	constexpr bool IsNone(Id id)
	{
		return id.GetVersion() == NoIdVersion;
	}

	inline String ToString(Id id)
	{
		if (id == NoId)
		{
			return "NoId";
		}
		return Strings::Format("{}:{}", id.GetIndex(), id.GetVersion());
	}

	/**
	 * Resolve an entity id from an string.
	 * The expected format is {}:{} where first is the index and second is the version.
	 * If a context is provided, providing the index alone as a number will resolve its last
	 * valid version
	 */
	P_API Id IdFromString(String str, IdContext* context);
#pragma endregion Ids


#pragma region Components
	// clang-format off
	template<typename T>
	concept StoresLastModified = !IsEmpty<T> && HasAnyTypeStaticFlags<T>(TF_ECS_StoreLastModified)
	    && (IsCopyConstructible<T> || IsMoveConstructible<T>);
	// clang-format on

	template<typename T>
	struct CMdfdWithLast
	{
		P_STRUCT(CMdfdWithLast, TF_NotSerialized)

		P_PROP(Last)
		T Last;
	};

	struct CMdfdWithoutLast
	{};

	/**
	 * Modified component
	 * Optionally, a component can be marked modified when written. This can be used when
	 * filtering.
	 */
	template<typename T>
	struct CMdfd : public Select<StoresLastModified<T>, CMdfdWithLast<T>, CMdfdWithoutLast>
	{
		P_STRUCT(CMdfd, TF_NotSerialized)

		using Type = T;
	};

	/**
	 * Removed component
	 * Optionally, an entity can be marked removed instead of actually removed and then flushed
	 * manually along with all other removed entities.
	 * @see RmId()
	 */
	struct P_API CRemoved
	{
		P_STRUCT(CRemoved, TF_NotSerialized)
	};

	struct P_API CParent
	{
		P_STRUCT(CParent)

		P_PROP(children, PF_Edit)
		TArray<Id> children;
	};
	P_API void Read(Reader& ct, CParent& val);
	P_API void Write(Writer& ct, const CParent& val);

	struct P_API CChild
	{
		P_STRUCT(CChild)

		P_PROP(parent, PF_Edit)
		Id parent = NoId;
	};
	P_API void Read(Reader& ct, CChild& val);
	P_API void Write(Writer& ct, const CChild& val);
#pragma endregion Components


	////////////////////////////////
	// COMPONENTS
	//
#pragma region Id Register
	/** IdRegistry tracks the existance and versioning of ids. Used internally by the ECS
	 * context */
	struct P_API IdRegistry
	{
		using Index   = Id::Index;
		using Version = Id::Version;

	private:
		TArray<Id> entities;
		TArray<Index> available;
		TArray<Id> deferredRemovals;    // List of ids that are invalid but not removed yet.

		Arena* arena = nullptr;

		// Support for multi-threaded entity creation and removal
		mutable std::shared_mutex mutex;


	public:

		IdRegistry(Arena& arena = GetCurrentArena())
		    : entities{arena}, available{arena}, deferredRemovals{arena}, arena{&arena}
		{}
		IdRegistry(IdRegistry&& other);
		IdRegistry(const IdRegistry& other);
		IdRegistry& operator=(IdRegistry&& other);
		IdRegistry& operator=(const IdRegistry& other);


		Id Create();
		void Create(TView<Id> newIds);
		bool RemoveInstant(TView<const Id> ids);
		bool Remove(TView<const Id> ids);
		bool FlushDeferredRemovals();
		const TArray<Id>& GetDeferredRemovals() const
		{
			return deferredRemovals;
		}
		bool IsValid(Id id) const;
		bool WasRemoved(Id id) const;
		TOptional<Version> GetValidVersion(Index idx) const;

		u32 Size() const;

		template<typename Callback>
		void Each(Callback cb) const;
	};
#pragma endregion Id Register


////////////////////////////////
// SERIALIZATION
//
#pragma region Serialization

	// Mark an entity as not serialized (it will be entirely ignored by the serializer)
	struct CNotSerialized
	{};

	class P_API EntityReader : public Reader
	{
		using Super = Reader;
		P_STRUCT(EntityReader)

		IdContext& context;

		// While serializing we create ids as Ids appear and link them.
		TArray<Id> ids;
		bool serializingMany = false;


	public:
		EntityReader(const p::Reader& parent, IdContext& context) : Reader(parent), context{context}
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
		IdContext& GetContext();

	protected:
		TypeId ProvideTypeId() const override
		{
			return p::GetTypeId<EntityReader>();
		}
	};


	class P_API EntityWriter : public Writer
	{
		using Super = Writer;
		P_STRUCT(EntityWriter)

		IdContext& context;

		// While serializing we create ids as Ids appear and link them.
		TArray<Id> ids;
		TMap<Id, i32> idToIndexes;
		bool serializingMany = false;


	public:
		EntityWriter(const Writer& parent, IdContext& context) : Writer(parent), context{context} {}

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

	protected:
		TypeId ProvideTypeId() const override
		{
			return p::GetTypeId<EntityWriter>();
		}
	};

	void P_API Read(p::Reader& ct, p::Id& val);
	void P_API Write(p::Writer& ct, p::Id val);
#pragma endregion Serialization


////////////////////////////////
// POOLS
//
#pragma region Pools
	enum class PoolRemovePolicy : u8
	{
		Swap,
		InPlace
	};

	/** Iterates the Ids contained in a pool */
	struct P_API PoolIterator final
	{
		using difference_type   = typename Id::Difference;
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

	struct P_API IPool
	{
		using Index           = Id::Index;
		using Iterator        = PoolIterator;
		using ReverseIterator = std::reverse_iterator<Iterator>;

	protected:
		TypeId typeId;


	public:
		IPool(TypeId typeId) : typeId{typeId} {}
		virtual ~IPool() {}

		// Returns the data pointer of a component if contained
		virtual bool Has(Id id) const                  = 0;
		virtual i32 Size() const                       = 0;
		virtual void* AddDefault(Id id)                = 0;
		virtual bool Remove(Id id)                     = 0;
		virtual void RemoveUnsafe(Id id)               = 0;
		virtual i32 Remove(TView<const Id> ids)        = 0;
		virtual void RemoveUnsafe(TView<const Id> ids) = 0;
		virtual void* TryGetVoid(Id id)                = 0;
		virtual void Clear()                           = 0;
		virtual TUniquePtr<IPool> Clone()              = 0;
		virtual const TArray<Id>& GetIdList() const    = 0;

		bool IsEmpty() const
		{
			return Size() > 0;
		}

		TypeId GetTypeId() const
		{
			return typeId;
		}

		// Standard functions:
	public:

		Iterator begin() const;
		Iterator end() const;
		Iterator cbegin() const;
		Iterator cend() const;
		ReverseIterator rbegin() const;
		ReverseIterator rend() const;
		ReverseIterator crbegin() const;
		ReverseIterator crend() const;
	};

	i32 GetSmallestPool(TView<const IPool* const> pools);

	struct P_API ComponentPool : public IPool
	{
	protected:
		TPageBuffer<i32, 4096> idIndices;
		TArray<Id> idList;
		Arena* arena         = nullptr;
		i32 lastRemovedIndex = NO_INDEX;
		PoolRemovePolicy removePolicy;


		ComponentPool(TypeId typeId, PoolRemovePolicy removePolicy, Arena& arena);
		ComponentPool(const ComponentPool& other);
		ComponentPool(ComponentPool&& other) noexcept;
		ComponentPool& operator=(const ComponentPool& other) noexcept;
		ComponentPool& operator=(ComponentPool&& other) noexcept;


	public:
		virtual ~ComponentPool() {}


		inline i32 GetIndexFromId(const Index index) const
		{
			return idIndices[index];
		}
		inline i32 GetIndexFromId(const Id id) const
		{
			return GetIndexFromId(id.GetIndex());
		}
		Id GetIdFromIndex(i32 index) const
		{
			return index < idList.Size() ? idList[index] : NoId;
		}

		bool Has(Id id) const override
		{
			const i32* const index = idIndices.At(id.GetIndex());
			return index && *index != NO_INDEX;
		}

		Iterator Find(const Id id) const
		{
			const i32* const index = idIndices.At(id.GetIndex());
			return index && *index != NO_INDEX ? Iterator{idList, *index} : end();
		}

		i32 Size() const override
		{
			return idList.Size();
		}

		bool IsEmpty() const
		{
			return Size() > 0;
		}

		const TArray<Id>& GetIdList() const override
		{
			return idList;
		}

	protected:

		Index EmplaceId(const Id id, bool forceBack);

		void PopId(Id id);

		void PopSwapId(Id id);

		void ClearIds();

	private:

		void BindOnPageAllocated();
	};


	template<typename T>
	struct TPool : public ComponentPool
	{
	private:
		TPageBuffer<T, 1024> data;


	public:
		TPool(p::IdContext& ctx, Arena& arena = GetCurrentArena())
		    : ComponentPool(p::GetTypeId<T>(), PoolRemovePolicy::InPlace, arena), data{arena}
		{}
		TPool(const TPool& other) : ComponentPool(other), data{*other.arena}
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
		TPool& operator=(const TPool& other)
		{
			ComponentPool::operator=(other);
			data = TPageBuffer<T, 1024>{*other.arena};

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
			return *this;
		}
		~TPool() override
		{
			Clear();
		}

		void* AddDefault(Id id) override
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
			if (Has(id))
			{
				if constexpr (!p::IsEmpty<T>)
				{
					return (Get(id) = T{p::Forward<Args>(args)...});
				}
			}
			else
			{
				const auto index = EmplaceId(id, false);
				if constexpr (!p::IsEmpty<T>)
				{
					data.Reserve(index + 1u);
					T* const value = data.Insert(index, p::Forward<Args>(args)...);
					return *value;
				}
			}
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
			P_Check(Has(id));
			if (removePolicy == PoolRemovePolicy::InPlace)
			{
				Pop(id);
			}
			else
			{
				PopSwap(id);
			}
		}

		i32 Remove(TView<const Id> ids) override
		{
			i32 removed = 0;
			if (removePolicy == PoolRemovePolicy::InPlace)
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

		void RemoveUnsafe(TView<const Id> ids) override
		{
			if (removePolicy == PoolRemovePolicy::InPlace)
			{
				for (Id id : ids)
				{
					P_Check(Has(id));
					Pop(id);
				}
			}
			else
			{
				for (Id id : ids)
				{
					P_Check(Has(id));
					PopSwap(id);
				}
			}
		}

		T& Get(Id id) requires(!p::IsEmpty<T>)
		{
			P_Check(Has(id));
			const i32 index = GetIndexFromId(id);
			return data[index];
		}

		const T& Get(Id id) const requires(!p::IsEmpty<T>)
		{
			P_Check(Has(id));
			return data[GetIndexFromId(id)];
		}

		T* TryGet(Id id)
		{
			if constexpr (!p::IsEmpty<T>)
			{
				const i32* const index = idIndices.At(id.GetIndex());
				if (index && *index != NO_INDEX)    // Has(id)
				{
					return &data[*index];
				}
			}
			return nullptr;
		}

		const T* TryGet(Id id) const
		{
			if constexpr (!p::IsEmpty<T>)
			{
				const i32* const index = idIndices.At(id.GetIndex());
				if (index && *index != NO_INDEX)    // Has(id)
				{
					return &data[*index];
				}
			}
			return nullptr;
		}

		void* TryGetVoid(Id id) override
		{
			return TryGet(id);
		}

		TUniquePtr<IPool> Clone() override
		{
			return p::MakeUnique<TPool<T>>(*this);
		}

		void Reserve(sizet size)
		{
			idList.Reserve(size);
			if constexpr (!p::IsEmpty<T>)
			{
				if (size > Size())
				{
					data.Reserve(size);
				}
			}
		}

		void ReserveMore(sizet size)
		{
			Reserve(Size() + size);
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
					if (idList[i].GetVersion() != NoIdVersion)
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
			for (; from && idList[from - 1].GetVersion() == NoIdVersion; --from) {}

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

					idIndices[listTo.GetIndex()] = to;
					to                           = from;

					for (; from && idList[from - 1].GetVersion() == NoIdVersion; --from) {}
				}
			}

			lastRemovedIndex = NO_INDEX;
			idList.Resize(from);
		}

		void Swap(const Id a, const Id b)
		{
			P_CheckMsg(Has(a), "Set does not contain entity");
			P_CheckMsg(Has(b), "Set does not contain entity");

			i32& aListIdx = idIndices[a.GetIndex()];
			i32& bListIdx = idIndices[b.GetIndex()];

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

	// CRemoved pool is special in the sense that it acts as an interface to deferred removals in
	// the IdRegistry
	template<>
	struct P_API TPool<CRemoved> : public IPool
	{
		friend IdContext;
		using T = CRemoved;

		p::IdRegistry* idRegistry = nullptr;

	public:
		TPool(p::IdContext& ctx, Arena& arena = GetCurrentArena());
		TPool(const TPool& other) = delete;
		TPool(TPool&& other)      = delete;
		TPool& operator=(const TPool& other)
		{
			return *this;
		}

		bool Has(Id id) const override
		{
			return GetIdList().ContainsSorted(id);
		}

		i32 Size() const override;

		bool IsEmpty() const
		{
			return Size() > 0;
		}

		void* AddDefault(Id id) override
		{
			Add(id, {});
			return nullptr;
		}

		void Add(Id id, CRemoved);

		template<typename It>
		void Add(It first, It last, const T& value = {}) requires(IsCopyConstructible<T>)
		{}

		template<typename It, typename CIt>
		void Add(It first, It last, CIt from)
		    requires(IsSame<std::decay_t<typename std::iterator_traits<CIt>::value_type>, T>)
		{}

		bool Remove(Id id) override
		{
			return false;
		}

		void RemoveUnsafe(Id id) override {}

		i32 Remove(TView<const Id> ids) override
		{
			return 0;
		}

		void RemoveUnsafe(TView<const Id> ids) override {}

		T* TryGet(Id id)
		{
			return nullptr;
		}

		const T* TryGet(Id id) const
		{
			return nullptr;
		}

		void* TryGetVoid(Id id) override
		{
			return nullptr;
		}

		TUniquePtr<IPool> Clone() override
		{
			return {};    // Can not clone
		}

		void Clear() override {}

		const TArray<Id>& GetIdList() const override;
	};


	struct P_API PoolInstance
	{
		TypeId componentId{};
		TUniquePtr<IPool> pool;

		PoolInstance(TypeId componentId, TUniquePtr<IPool>&& pool);
		PoolInstance(PoolInstance&& other) noexcept;
		explicit PoolInstance(const PoolInstance& other);
		PoolInstance& operator=(PoolInstance&& other) noexcept;
		PoolInstance& operator=(const PoolInstance& other);
		TypeId GetId() const;
		IPool* GetPool() const;
		bool operator<(const PoolInstance& other) const;
	};
#pragma endregion Pools


////////////////////////////////
// OPERATIONS
//
#pragma region Operations
	/**
	 * IdOperations contains shared logic to view and edit components and statics in Contexts and
	 * Scopes This class assumes GetPool and AssurePool are present in Parent
	 */
	template<typename Parent>
	struct TIdOperations
	{
		const Parent& AsParent() const
		{
			return *static_cast<const Parent*>(this);
		}

		template<typename Component>
		CopyConst<TPool<Mut<Component>>, Component>& AssurePool() const
		{
			return AsParent().template AssurePool<Component>();
		}

		template<typename Component>
		CopyConst<TPool<Mut<Component>>, Component>* GetPool() const
		{
			return AsParent().template GetPool<Component>();
		}

		IdContext& GetContext() const
		{
			return AsParent().GetContext();
		}

		template<typename Component>
		i32 Size() const
		{
			return GetPool<const Component>()->Size();
		}

		bool IsValid(Id id) const
		{
			return GetContext().IsValid(id);
		}

		template<typename... Component>
		bool Has(Id id) const requires(sizeof...(Component) >= 1)
		{
			return (GetPool<const Component>()->Has(id) && ...);
		}

		template<typename Component>
		void MarkModified(TView<const Id> ids, const TPool<Component>* pool = nullptr) const
		{
			auto& mdfdPool = AssurePool<CMdfd<Component>>();
			if constexpr (StoresLastModified<Component>)
			{
				if (!pool)
				{
					pool = GetPool<const Component>();
					if (!pool)
					{
						return;
					}
				}

				mdfdPool.ReserveMore(ids.Size());
				for (Id id : ids)
				{
					if (!mdfdPool.Has(id))    // If we store value, we only add if it it wasn't
					                          // modified already
					{
						Component* comp = pool->template TryGet<Component>(id);
						if constexpr (IsMoveConstructible<Component>)
						{
							mdfdPool.Add(
							    id, comp ? CMdfd<Component>{p::Move(*comp)} : CMdfd<Component>{});
						}
						else
						{
							mdfdPool.Add(id, comp ? CMdfd<Component>{*comp} : CMdfd<Component>{});
						}
					}
				}
			}
			else
			{
				mdfdPool.Add(ids.begin(), ids.end(), CMdfd<Component>{});
			}
		}

		template<typename Component>
		bool IsModified(Id id) const
		{
			return Has<CMdfd<Mut<Component>>>(id);
		}

		template<typename Component>
		decltype(auto) Add(Id id, Component&& value = {}) const requires(IsMutable<Component>)
		{
			P_Check(IsValid(id));
			auto& pool = AssurePool<Component>();
			if constexpr (HasAnyTypeStaticFlags<Component>(TF_ECS_AutoModify))
			{
				MarkModified<Component>(id, &pool);
			}
			return pool.Add(id, p::Forward<Component>(value));
		}
		template<typename Component>
		decltype(auto) Add(Id id, const Component& value) const requires(IsMutable<Component>)
		{
			P_Check(IsValid(id));
			auto& pool = AssurePool<Component>();
			if constexpr (HasAnyTypeStaticFlags<Component>(TF_ECS_AutoModify))
			{
				MarkModified<Component>(id, &pool);
			}
			return pool.Add(id, value);
		}

		// Add component to an entities (if they dont have it already)
		template<typename... Component>
		void Add(Id id) const requires((IsMutable<Component> && ...) && sizeof...(Component) > 1)
		{
			(Add<Component>(id), ...);
		}

		// Add component to many entities (if they dont have it already)
		template<typename Component>
		decltype(auto) AddN(TView<const Id> ids, const Component& value = {}) const
		{
			auto& pool = AssurePool<Component>();
			if constexpr (HasAnyTypeStaticFlags<Component>(TF_ECS_AutoModify))
			{
				MarkModified<Component>(ids, &pool);
			}
			return pool.Add(ids.begin(), ids.end(), value);
		}

		template<typename Component>
		void AddN(TView<const Id> ids, const TView<const Component>& values)
		{
			P_Check(ids.Size() == values.Size());
			auto& pool = AssurePool<Component>();
			if constexpr (HasAnyTypeStaticFlags<Component>(TF_ECS_AutoModify))
			{
				MarkModified<Component>(ids, &pool);
			}
			pool.Add(ids.begin(), ids.end(), values.begin());
		}

		// Add components to many entities (if they dont have it already)
		template<typename... Component>
		void AddN(TView<const Id> ids) const
		    requires((IsMutable<Component> && ...) && sizeof...(Component) > 1)
		{
			(AddN<Component>(ids), ...);
		}


		template<typename Component>
		void Remove(const Id id) const requires(IsMutable<Component>)
		{
			if (auto* pool = GetPool<Component>())
			{
				if constexpr (HasAnyTypeStaticFlags<Component>(TF_ECS_AutoModify))
				{
					MarkModified<Component>(id, pool);
				}
				pool->Remove(id);
			}
		}
		template<typename... Component>
		void Remove(const Id id) const requires(sizeof...(Component) > 1)
		{
			(Remove<Component>(id), ...);
		}
		template<typename Component>
		void Remove(TView<const Id> ids) const requires(IsMutable<Component>)
		{
			if (auto* pool = GetPool<Component>())
			{
				if constexpr (HasAnyTypeStaticFlags<Component>(TF_ECS_AutoModify))
				{
					MarkModified<Component>(ids, pool);
				}
				pool->Remove(ids);
			}
		}
		template<typename... Component>
		void Remove(TView<const Id> ids) const requires(sizeof...(Component) > 1)
		{
			(Remove<Component>(ids), ...);
		}

		template<typename Component>
		Component& Get(Id id) const
		{
			auto* const pool = GetPool<Component>();
			P_Check(pool);
			if constexpr (IsMutable<Component>
			              && HasAnyTypeStaticFlags<Component>(TF_ECS_AutoModify))
			{
				MarkModified<Component>(id, pool);
			}
			return pool->Get(id);
		}

		template<typename Component>
		Component* TryGet(Id id) const
		{
			auto* const pool = GetPool<Component>();
			P_Check(pool);
			Component* value = pool->TryGet(id);
			if constexpr (IsMutable<Component>
			              && HasAnyTypeStaticFlags<Component>(TF_ECS_AutoModify))
			{
				if (value)
				{
					MarkModified<Component>(id, pool);
				}
			}
			return value;
		}

		template<typename Component>
		Component& GetOrAdd(Id id) const requires(IsMutable<Component>)
		{
			auto& pool = AssurePool<Component>();

			if constexpr (HasAnyTypeStaticFlags<Component>(TF_ECS_AutoModify))
			{
				MarkModified<Component>(id, *pool);
			}

			if (pool.Has(id))
			{
				return pool.Get(id);
			}
			return pool.Add(id);
		}

		template<typename... Component>
		void ClearPool() const
		{
			(GetPool<Component>()->Clear(), ...);
		}
	};
#pragma endregion Operations


////////////////////////////////
// CONTEXT
//
#pragma region Context
	enum class IdRemovePolicy : u8
	{
		Instant,
		Deferred
	};

	struct P_API SortLessStatics
	{
		bool operator()(const OwnPtr& a, const OwnPtr& b) const
		{
			return a.GetId() < b.GetId();
		}    // namespace p

		bool operator()(TypeId a, const OwnPtr& b) const
		{
			return a < b.GetId();
		}

		bool operator()(const OwnPtr& a, TypeId b) const
		{
			return a.GetId() < b;
		}
	};

	struct P_API IdContext : public TIdOperations<IdContext>
	{
	private:
		IdRegistry idRegistry;
		mutable TArray<PoolInstance> pools;
		TArray<OwnPtr> statics;
		IdRemovePolicy removePolicy = IdRemovePolicy::Instant;


	public:
		IdContext();
		~IdContext()
		{
			Reset();
		}
		explicit IdContext(const IdContext& other) noexcept;
		explicit IdContext(IdContext&& other) noexcept;
		IdContext& operator=(const IdContext& other) noexcept;
		IdContext& operator=(IdContext&& other) noexcept;

#pragma region Entities
		// Reflection helpers
		void* AddByTypeId(TypeId typeId, Id id);
		void RemoveByTypeId(TypeId typeId, Id id);

		const IdRegistry& GetIdRegistry() const
		{
			return idRegistry;
		}
		IdRegistry& GetIdRegistry()
		{
			return idRegistry;
		}
		bool IsValid(Id id) const;
		bool WasRemoved(Id id) const;
		bool IsOrphan(const Id id) const;

		template<typename Callback>
		void Each(Callback cb) const
		{
			idRegistry.Each(cb);
		}

		u32 Size() const
		{
			return idRegistry.Size();
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

		IdContext& GetContext() const
		{
			return *const_cast<IdContext*>(this);
		}

		// Finds or creates a pool
		template<typename T>
		TPool<Mut<T>>& AssurePool() const;

		IPool* GetPool(TypeId typeId) const;
		void GetPools(TView<const TypeId> typeIds, TArray<IPool*>& outPools) const;
		void GetPools(TView<const TypeId> typeIds, TArray<const IPool*>& outPools) const
		{
			GetPools(typeIds, reinterpret_cast<TArray<IPool*>&>(outPools));
		}

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
		Static& SetStatic();
		template<typename Static>
		Static& SetStatic(Static&& value);
		template<typename Static>
		Static& SetStatic(const Static& value);
		template<typename Static>
		Static& GetOrSetStatic();
		template<typename Static>
		Static& GetOrSetStatic(Static&& newValue);
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
		void CopyFrom(const IdContext& other);
		void MoveFrom(IdContext&& other);

		static OwnPtr& FindOrAddStaticPtr(
		    TArray<OwnPtr>& statics, const TypeId typeId, bool* bAdded = nullptr);

		template<typename T>
		PoolInstance CreatePoolInstance() const;
	};
#pragma endregion Context


////////////////////////////////
// ID SCOPES
//
#pragma region IdScopes
	template<typename T>
	struct TIsAutoModified
	{
		static constexpr bool value = HasAnyTypeStaticFlags<T>(TF_ECS_AutoModify);
	};


	// Base for TIdScope. Assumes all types are mutable (not const)
	template<typename W, typename... R>
	struct TIdScopeBase : public TIdOperations<TIdScopeBase<W, R...>>
	{
		template<typename W2, typename... R2>
		friend struct TIdScopeBase;

	private:
		using ModifyWrites = W::template Filter<TIsAutoModified>::template Wrap<CMdfd>;
		using ModifyReads = TTypeList<R...>::template Filter<TIsAutoModified>::template Wrap<CMdfd>;

	public:
		using WDependencies  = W::template Append<ModifyWrites>;
		using RDependencies  = TTypeList<R...>::template Append<ModifyReads>;
		using RWDependencies = RDependencies::template Append<WDependencies>::Deduplicate;
		using Pools          = RWDependencies::template WrapPtr<TPool>;
		using Tuple          = Pools::template To<std::tuple>;

	protected:
		IdContext& context;
		Tuple pools;

	public:
		TIdScopeBase(IdContext& context)
		    : context{context}, pools(RWDependencies::Call([&context]<typename... T> {
			    return Tuple{&context.AssurePool<T>()...};
		    }))
		{}
		TIdScopeBase(const TIdScopeBase& other) : context{other.context}, pools{other.pools} {}

		// Construct a child scope (super-set) from another scope
		template<typename... T2>
		TIdScopeBase(const TIdScopeBase<T2...>& other) : context{other.context}
		{
			using Other = TIdScopeBase<T2...>;

			constexpr bool validReads = RWDependencies::Call([]<typename... T>() {
				return (Other::template IsReadable<T>() && ...);
			});

			constexpr bool validWrites = WDependencies::Call([]<typename... T>() {
				return (Other::template IsWritable<T>() && ...);
			});

			static_assert(
			    validReads, "Parent scope lacks read dependencies required by this scope.");
			static_assert(
			    validWrites, "Parent scope lacks *write* dependencies required by this scope.");

			// Prevent compiler errors, we already have static_asserts
			if constexpr (validReads && validWrites)
			{
				pools = RWDependencies::Call([&other]<typename... T> {
					return Tuple{std::get<TPool<T>*>(other.pools)...};
				});
			}
		}


		template<typename Component>
		TPool<Mut<Component>>* GetPool() const requires(IsMutable<Component>)
		{
			static_assert(IsWritable<Component>(), "Can't modify components of this type");
			if constexpr (IsWritable<Component>())    // Prevent missleading errors if condition
			                                          // fails
			{
				return std::get<TPool<Mut<Component>>*>(pools);
			}
			else
			{
				return nullptr;
			}
		}

		template<typename Component>
		const TPool<Mut<Component>>* GetPool() const requires(IsConst<Component>)
		{
			static_assert(IsReadable<Component>(), "Can't read components of this type");
			if constexpr (IsReadable<Component>())    // Prevent missleading errors if condition
			                                          // fails
			{
				return std::get<TPool<Mut<Component>>*>(pools);
			}
			else
			{
				return nullptr;
			}
		}

		template<typename Component>
		TPool<Component>& AssurePool() const requires(IsMutable<Component>)
		{
			return *GetPool<Component>();
		}

		template<typename Component>
		const TPool<Mut<Component>>& AssurePool() const requires(IsConst<Component>)
		{
			return *GetPool<Component>();
		}


		IdContext& GetContext() const
		{
			return context;
		}

		template<typename Component>
		static constexpr bool IsMdfdType() requires(!HasTypeMember<Component>)
		{
			return false;
		}
		template<typename Component>
		static constexpr bool IsMdfdType() requires(HasTypeMember<Component>)
		{
			return IsSame<CMdfd<typename Component::Type>, Component>;
		}

		template<typename Component>
		static constexpr bool IsReadable()
		{
			return RWDependencies::template Contains<Mut<Component>>();
		}

		template<typename Component>
		static constexpr bool IsWritable()
		{
			return IsMutable<Component> && WDependencies::template Contains<Component>();
		}
	};


	template<typename... T>
	struct Writes : public TTypeList<T...>
	{};

	template<typename... R>
	struct TIdScope : public TIdScopeBase<Writes<>, Mut<R>...>
	{
		using TIdScopeBase<Writes<>, Mut<R>...>::TIdScopeBase;
	};

	template<typename... W, typename... R>
	struct TIdScope<Writes<W...>, R...> : public TIdScopeBase<Writes<Mut<W>...>, Mut<R>...>
	{
		using TIdScopeBase<Writes<Mut<W>...>, Mut<R>...>::TIdScopeBase;
	};


	template<typename... T>
	using TIdScopeRef = const TIdScope<T...>&;

	struct IdScope : public TIdOperations<IdScope>
	{
	protected:

		IdContext& context;
		// TArray<TypeAccess> types;
		TArray<IPool*> pools;


	public:
		IdScope(IdContext& ctx, const TArray<TypeId>& types) : context{ctx} {}

		template<typename... T>
		IdScope(TIdScopeRef<T...> scope) : context{scope.context}
		{}

		template<typename Component>
		TPool<Mut<Component>>* GetPool() const requires(IsMutable<Component>)
		{
			return nullptr;
		}

		template<typename Component>
		const TPool<Mut<Component>>* GetPool() const requires(IsConst<Component>)
		{
			return nullptr;
		}

	private:

		i32 GetPoolIndex() const
		{
			return 0;
		}
	};
#pragma endregion IdScopees


////////////////////////////////
// FILTERING
//
#pragma region Filtering

	/** Remove ids containing a component from 'ids'. Does not guarantee order. */
	P_API void ExcludeIdsWith(const IPool* pool, TArray<Id>& ids, const bool shouldShrink = true);

	/** Remove ids containing a component from 'ids'. Guarantees order. */
	P_API void ExcludeIdsWithStable(
	    const IPool* pool, TArray<Id>& ids, const bool shouldShrink = true);

	/** Remove ids NOT containing a component from 'ids'. Does not guarantee order. */
	P_API void ExcludeIdsWithout(
	    const IPool* pool, TArray<Id>& ids, const bool shouldShrink = true);

	/** Remove ids NOT containing a component from 'ids'. Guarantees order. */
	P_API void ExcludeIdsWithoutStable(
	    const IPool* pool, TArray<Id>& ids, const bool shouldShrink = true);


	/** Find ids containing a component from a list 'source' into 'results'. */
	P_API void FindIdsWith(const IPool* pool, TView<const Id> source, TArray<Id>& results);
	P_API void FindIdsWith(
	    TView<const IPool* const> pools, TView<const Id> source, TArray<Id>& results);

	/** Find ids NOT containing a component from a list 'source' into 'results'. */
	P_API void FindIdsWithout(const IPool* pool, TView<const Id> source, TArray<Id>& results);


	/**
	 * Find and remove ids containing a component from list 'source' into 'results'.
	 * Does not guarantee order.
	 */
	P_API void ExtractIdsWith(
	    const IPool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink = true);

	/**
	 * Find and remove ids containing a component from list 'source' into 'results'.
	 * Guarantees order.
	 */
	P_API void ExtractIdsWithStable(
	    const IPool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink = true);

	/**
	 * Find and remove ids containing a component from list 'source' into 'results'.
	 * Does not guarantee order.
	 */
	P_API void ExtractIdsWithout(
	    const IPool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink = true);

	/**
	 * Find and remove ids not containing a component from list 'source' into 'results'.
	 * Guarantees order.
	 */
	P_API void ExtractIdsWithoutStable(
	    const IPool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink = true);


	/** Find all ids containing all of the components */
	P_API void FindAllIdsWith(TView<const IPool* const> pools, TArray<Id>& ids);

	/** Find all ids containing any of the components. Includes possible duplicates */
	P_API void FindAllIdsWithAny(TView<const IPool* const> pools, TArray<Id>& ids);

	/** Find all ids containing any of the components. Prevents duplicates */
	P_API void FindAllIdsWithAnyUnique(TView<const IPool* const> pools, TArray<Id>& ids);

	P_API Id GetFirstIdWith(TView<const IPool* const> pools);


	// Templated API

	/**
	 * Remove ids containing a component from 'ids'. Does not guarantee order.
	 *
	 * @param scope from where to scope pools
	 * @param ids array that will be modified
	 * @param shouldShrink if true, the ids array will be shrink at the end
	 * @see ExcludeIdsWithStable(), ExcludeIdsWithout()
	 */
	template<typename Component, typename Scope>
	void ExcludeIdsWith(const Scope& scope, TArray<Id>& ids, const bool shouldShrink = true)
	{
		ExcludeIdsWith(&scope.template AssurePool<const Component>(), ids, shouldShrink);
	}
	template<typename... Component, typename Scope>
	void ExcludeIdsWith(const Scope& scope, TArray<Id>& ids, const bool shouldShrink = true)
	    requires(sizeof...(Component) > 1)
	{
		(ExcludeIdsWith<Component>(scope, ids, shouldShrink), ...);
	}

	template<typename Predicate>
	void ExcludeIdsWith(TArray<Id>& ids, Predicate predicate, const bool shouldShrink = true)
	{
		for (i32 i = ids.Size() - 1; i >= 0; --i)
		{
			if (predicate(ids[i]))
			{
				ids.RemoveAtSwapUnsafe(i);
			}
		}
		if (shouldShrink)
		{
			ids.Shrink();
		}
	}

	/**
	 * Remove ids containing a component from 'ids'. Guarantees order.
	 *
	 * @param scope from where to scope pools
	 * @param ids array that will be modified
	 * @param shouldShrink if true, the ids array will be shrink at the end
	 * @see ExcludeIdsWith(), ExcludeIdsWithoutStable()
	 */
	template<typename Component, typename Scope>
	void ExcludeIdsWithStable(const Scope& scope, TArray<Id>& ids, const bool shouldShrink = true)
	{
		ExcludeIdsWithStable(&scope.template AssurePool<const Component>(), ids, shouldShrink);
	}
	template<typename... Component, typename Scope>
	void ExcludeIdsWithStable(const Scope& scope, TArray<Id>& ids, const bool shouldShrink = true)
	    requires(sizeof...(Component) > 1)
	{
		(ExcludeIdsWithStable<Component>(scope, ids, shouldShrink), ...);
	}

	/**
	 * Remove ids NOT containing a component from 'ids'. Does not guarantee order.
	 *
	 * @param scope from where to scope pools
	 * @param ids array that will be modified
	 * @param shouldShrink if true, the ids array will be shrink at the end
	 * @see ExcludeIdsWithoutStable(), ExcludeIdsWith()
	 */
	template<typename Component, typename Scope>
	void ExcludeIdsWithout(const Scope& scope, TArray<Id>& ids, const bool shouldShrink = true)
	{
		ExcludeIdsWithout(&scope.template AssurePool<const Component>(), ids, shouldShrink);
	}

	template<typename... Component, typename Scope>
	void ExcludeIdsWithout(const Scope& scope, TArray<Id>& ids, const bool shouldShrink = true)
	    requires(sizeof...(Component) > 1)
	{
		(ExcludeIdsWithout<Component>(scope, ids, shouldShrink), ...);
	}

	/**
	 * Remove ids NOT containing a component from 'ids'. Guarantees order.
	 *
	 * @param scope from where to scope pools
	 * @param ids array that will be modified
	 * @param shouldShrink if true, the ids array will be shrink at the end
	 * @see ExcludeIdsWithout(), ExcludeIdsWithStable()
	 */
	template<typename Component, typename Scope>
	void ExcludeIdsWithoutStable(
	    const Scope& scope, TArray<Id>& ids, const bool shouldShrink = true)
	{
		ExcludeIdsWithoutStable(&scope.template AssurePool<const Component>(), ids, shouldShrink);
	}
	template<typename... Component, typename Scope>
	void ExcludeIdsWithoutStable(const Scope& scope, TArray<Id>& ids,
	    const bool shouldShrink = true) requires(sizeof...(Component) > 1)
	{
		(ExcludeIdsWithoutStable<Component>(scope, ids, shouldShrink), ...);
	}

	/**
	 * Remove ids that are invalid.
	 *
	 * @param scope
	 * @param ids array that will be modified
	 * @param shouldShrink if true, the ids array will be shrink at the end
	 * @see ExcludeIdsInvalidStable()
	 */
	template<typename Scope>
	void ExcludeIdsInvalid(const Scope& scope, TArray<Id>& ids, const bool shouldShrink = true)
	{
		ids.RemoveIfSwap(
		    [&scope](Id id) {
			return !scope.IsValid(id);
		    },
		    shouldShrink);
	}

	/**
	 * Remove ids that are invalid. Guarantees order.
	 *
	 * @param scope
	 * @param ids array that will be modified
	 * @param shouldShrink if true, the ids array will be shrink at the end
	 * @see ExcludeIdsInvalid()
	 */
	template<typename Scope>
	void ExcludeIdsInvalidStable(
	    const Scope& scope, TArray<Id>& ids, const bool shouldShrink = true)
	{
		ids.RemoveIf(
		    [&scope](Id id) {
			return !scope.IsValid(id);
		    },
		    shouldShrink);
	}


	/** Find ids containing a component from a list 'source' into 'results'. */
	template<typename Component, typename Scope>
	void FindIdsWith(const Scope& scope, const TView<Id>& source, TArray<Id>& results)
	{
		FindIdsWith(&scope.template AssurePool<const Component>(), source, results);
	}
	template<typename... Component, typename Scope>
	void FindIdsWith(const Scope& scope, const TView<Id>& source, TArray<Id>& results)
	    requires(sizeof...(Component) > 1)
	{
		FindIdsWith({&scope.template AssurePool<const Component>()...}, source, results);
	}

	template<typename... Component, typename Scope>
	TArray<Id> FindIdsWith(const Scope& scope, const TView<Id>& source)
	{
		TArray<Id> results;
		FindIdsWith<Component...>(scope, source, results);
		return Move(results);
	}

	/** Find ids NOT containing a component from a list 'source' into 'results'. */
	template<typename Component, typename Scope>
	void FindIdsWithout(const Scope& scope, const TArray<Id>& source, TArray<Id>& results)
	{
		FindIdsWithout(&scope.template AssurePool<const Component>(), source, results);
	}
	template<typename Component, typename Scope>
	TArray<Id> FindIdsWithout(const Scope& scope, const TArray<Id>& source)
	{
		TArray<Id> results;
		FindIdsWithout<Component>(scope, source, results);
		return Move(results);
	}

	/**
	 * Find and remove ids containing a component from list 'source' into 'results'.
	 * Does not guarantee order.
	 */
	template<typename Component, typename Scope>
	void ExtractIdsWith(
	    const Scope& scope, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink = true)
	{
		ExtractIdsWith(&scope.template AssurePool<const Component>(), source, results);
	}
	template<typename Component, typename Scope>
	TArray<Id> ExtractIdsWith(
	    const Scope& scope, TArray<Id>& source, const bool shouldShrink = true)
	{
		TArray<Id> results;
		ExtractIdsWith<Component>(scope, source, results);
		return Move(results);
	}

	/**
	 * Find and remove ids containing a component from list 'source' into 'results'.
	 * Guarantees order.
	 */
	template<typename Component, typename Scope>
	void ExtractIdsWithStable(
	    const Scope& scope, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink = true)
	{
		ExtractIdsWithStable(&scope.template AssurePool<const Component>(), source, results);
	}
	template<typename Component, typename Scope>
	TArray<Id> ExtractIdsWithStable(
	    const Scope& scope, TArray<Id>& source, const bool shouldShrink = true)
	{
		TArray<Id> results;
		ExtractIdsWithStable<Component>(scope, source, results);
		return Move(results);
	}

	/**
	 * Find and remove ids containing a component from list 'source' into 'results'.
	 * Does not guarantee order.
	 */
	template<typename Component, typename Scope>
	void ExtractIdsWithout(
	    const Scope& scope, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink = true)
	{
		ExtractIdsWithout(&scope.template AssurePool<const Component>(), source, results);
	}
	template<typename Component, typename Scope>
	TArray<Id> ExtractIdsWithout(
	    const Scope& scope, TArray<Id>& source, const bool shouldShrink = true)
	{
		TArray<Id> results;
		ExtractIdsWithout<Component>(scope, source, results);
		return Move(results);
	}

	/**
	 * Find and remove ids not containing a component from list 'source' into 'results'.
	 * Guarantees order.
	 */
	template<typename Component, typename Scope>
	void ExtractIdsWithoutStable(
	    const Scope& scope, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink = true)
	{
		ExtractIdsWithoutStable(&scope.template AssurePool<const Component>(), source, results);
	}
	template<typename Component, typename Scope>
	TArray<Id> ExtractIdsWithoutStable(
	    const Scope& scope, TArray<Id>& source, const bool shouldShrink = true)
	{
		TArray<Id> results;
		ExtractIdsWithoutStable<Component>(scope, source, results);
		return Move(results);
	}


	/**
	 * Find all ids containing all of the components
	 *
	 * @param scope from where to scope pools
	 * @param ids array where matching ids will be added
	 * @see FindAllIdsWithAny()
	 */
	template<typename... Component, typename Scope>
	void FindAllIdsWith(const Scope& scope, TArray<Id>& ids) requires(sizeof...(Component) >= 1)
	{
		FindAllIdsWith({scope.template GetPool<const Component>()...}, ids);
	}


	/**
	 * Find all ids containing all of the components
	 *
	 * @param scope from where to scope pools
	 * @return ids array with matching ids
	 * @see FindAllIdsWithAny()
	 */
	template<typename... Component, typename Scope>
	TArray<Id> FindAllIdsWith(const Scope& scope) requires(sizeof...(Component) >= 1)
	{
		TArray<Id> ids;
		FindAllIdsWith<Component...>(scope, ids);
		return Move(ids);
	}

	/**
	 * Find all ids containing any of the components.
	 * Includes possible duplicates
	 *
	 * @param scope from where to scope pools
	 * @param ids array where matching ids will be added
	 * @see FindAllIdsWith()
	 */
	template<typename... Component, typename Scope>
	void FindAllIdsWithAny(const Scope& scope, TArray<Id>& ids) requires(sizeof...(Component) >= 1)
	{
		FindAllIdsWithAny({scope.template GetPool<const Component>()...}, ids);
	}

	/**
	 * Find all ids containing any of the components.
	 * Prevents duplicates
	 *
	 * @param scope from where to scope pools
	 * @param ids array where matching ids will be added
	 * @see FindAllIdsWithAnyUnique()
	 */
	template<typename... Component, typename Scope>
	void FindAllIdsWithAnyUnique(const Scope& scope, TArray<Id>& ids)
	    requires(sizeof...(Component) >= 1)
	{
		FindAllIdsWithAnyUnique({scope.template GetPool<const Component>()...}, ids);
	}

	/**
	 * Find all ids containing any of the components.
	 * Includes possible duplicates
	 *
	 * @param scope from where to scope pools
	 * @return ids array with matching ids
	 * @see FindAllIdsWith()
	 */
	template<typename... Component, typename Scope>
	TArray<Id> FindAllIdsWithAny(const Scope& scope) requires(sizeof...(Component) >= 1)
	{
		TArray<Id> ids;
		FindAllIdsWithAny<Component...>(scope, ids);
		return Move(ids);
	}

	/**
	 * Find all ids containing any of the components.
	 * Prevents duplicates
	 *
	 * @param scope from where to scope pools
	 * @return ids array with matching ids
	 * @see FindAllIdsWithAny()
	 */
	template<typename... Component, typename Scope>
	TArray<Id> FindAllIdsWithAnyUnique(const Scope& scope) requires(sizeof...(Component) >= 1)
	{
		TArray<Id> ids;
		FindAllIdsWithAnyUnique<Component...>(scope, ids);
		return Move(ids);
	}

	template<typename... Component, typename Scope>
	Id GetFirstIdWith(const Scope& scope)
	{
		return GetFirstIdWith({scope.template GetPool<const Component>()...});
	}
#pragma endregion Filtering


////////////////////////////////
// EDITING
//
#pragma region Editing

	// Create
	P_API Id AddId(IdContext& ctx);
	P_API void AddId(IdContext& ctx, TView<Id> Ids);

	// Remove
	P_API bool RmId(IdContext& ctx, TView<const Id> ids, RmIdFlags flags = RmIdFlags::None);
	P_API bool FlushDeferredRemovals(IdContext& ctx);
#pragma endregion Editing


////////////////////////////////
// HIERARCHY
//
#pragma region Hierarchy

	// Link a list of nodes at the end of the parent children list
	P_API void AttachId(
	    TIdScopeRef<Writes<CChild, CParent>> scope, Id parent, TView<const Id> children);
	// Link a list of nodes after prevChild in the list of children nodes
	P_API void AttachIdAfter(
	    TIdScopeRef<Writes<CChild, CParent>> scope, Id parent, TView<Id> childrenIds, Id prevChild);
	P_API void TransferIdChildren(
	    TIdScopeRef<Writes<CChild, CParent>> scope, TView<const Id> childrenIds, Id destination);
	// TODO: void TransferAllChildren(IdContext& context, Id origin, Id destination);
	P_API void DetachIdParent(TIdScopeRef<Writes<CParent, CChild>> scope,
	    TView<const Id> childrenIds, bool keepComponents);
	P_API void DetachIdChildren(TIdScopeRef<Writes<CParent, CChild>> scope, TView<const Id> parents,
	    bool keepComponents = false);

	/** Obtain direct children ids from the provided parent Id. Examples:
	 * - Children of A (where A->B->C) is B.
	 * - Children of A (where A->B, A->C) are B and C.
	 */
	P_API const TArray<Id>* GetIdChildren(TIdScopeRef<CParent> scope, Id node);

	/** Obtain direct children ids from the provided parent Ids. Examples:
	 * - Children of A (where A->B->C) is B.
	 * - Children of A (where A->B, A->C) are B and C.
	 * - Children of A and B (where A->B->C) are B, C.
	 */
	P_API void GetIdChildren(
	    TIdScopeRef<CParent> scope, TView<const Id> nodes, TArray<Id>& outChildrenIds);

	/** Obtain all children ids from the provided parent Ids. Examples:
	 * - All children of A (where A->B->C) are B and C.
	 * - All children of A and D (where A->B->C, D->E->F) are B, C, E and F.
	 * - All children of A and B (where A->B->C->D) are B, C, D, C, D (duplicates are not
	 * handled).
	 */
	P_API void GetAllIdChildren(TIdScopeRef<CParent> scope, TView<const Id> parentIds,
	    TArray<Id>& outChildrenIds, u32 depth = 1);

	P_API Id GetIdParent(TIdScopeRef<CChild> scope, Id childId);
	P_API void GetIdParent(
	    TIdScopeRef<CChild> scope, TView<const Id> childrenIds, TArray<Id>& outParents);
	P_API void GetAllIdParents(
	    TIdScopeRef<CChild> scope, TView<const Id> childrenIds, TArray<Id>& outParents);

	/**
	 * Find a parent id matching a delegate
	 */
	P_API Id FindIdParent(TIdScopeRef<CChild> scope, Id child, const TFunction<bool(Id)>& callback);
	P_API void FindIdParents(TIdScopeRef<CChild> scope, TView<const Id> childrenIds,
	    TArray<Id>& outParents, const TFunction<bool(Id)>& callback);

	// void Copy(IdContext& context, t TArray<Id>& nodes, TArray<Id>& outNewNodes);
	// void CopyDeep(IdContext& context, const TArray<Id>& rootNodes, TArray<Id>& outNewRootNodes);
	// void CopyAndTransferAllChildrenDeep(IdContext& context, Id root, Id otherRoot);

	/**
	 * Iterates children nodes making sure child->parent links are correct or fixed
	 * Only first depth links are affected
	 * Complexity: O(N)
	 * @parents: where to look for children to fix up
	 * @return true if an incorrect link was found and fixed
	 */
	P_API bool FixParentIdLinks(TIdScopeRef<Writes<CChild>, CParent> scope, TView<Id> parents);

	/**
	 * Iterates children nodes looking for invalid child->parent links
	 * Only first depth links are affected
	 * Complexity: O(1) <-> O(N) (First invalid link makes an early out)
	 * @parents: where to look for children
	 * @return true if an incorrect link was found
	 */
	P_API bool ValidateParentIdLinks(TIdScopeRef<CChild, CParent> scope, TView<Id> parents);

	P_API void GetRootIds(TIdScopeRef<CChild, CParent> scope, TArray<Id>& outRoots);
#pragma endregion Hierarchy


////////////////////////////////
// DEFINITIONS
//
#pragma region Definitions

	template<typename Callback>
	void IdRegistry::Each(Callback cb) const
	{
		if (available.IsEmpty() && deferredRemovals.IsEmpty())
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
				if (id.GetIndex() == i)
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
					const Id id = ids[i];
					key.clear();
					Strings::FormatTo(key, "{}", i);

					if (EnterNext(key))
					{
						if constexpr (!IsEmpty<T>)
						{
							T& comp = pool.Has(id) ? pool.Get(id) : pool.Add(id);
							Serialize(comp);
						}
						else
						{
							pool.Add(id);
						}
						Leave();
					}
				}
			}
			else
			{
				if constexpr (!IsEmpty<T>)
				{
					const Id id = ids[0];
					T& comp     = pool.Has(id) ? pool.Get(id) : pool.Add(id);
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
		TArray<TPair<i32, Id>> typeIds;    // TODO: Make sure this is needed

		auto* pool = context.GetPool<const T>();
		if (pool)
		{
			typeIds.Reserve(Min(i32(pool->Size()), ids.Size()));
			for (i32 i = 0; i < ids.Size(); ++i)
			{
				const Id id = ids[i];
				if (pool->Has(id))
				{
					typeIds.Add({i, id});
				}
			}
		}

		if (typeIds.IsEmpty())
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
				for (auto id : typeIds)
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
					Serialize(pool->Get(typeIds.First().second));
				}
			}
			Leave();
		}
		PopFlags();
	}


	template<typename T>
	inline TPool<Mut<T>>& IdContext::AssurePool() const
	{
		const TypeId componentId = RegisterTypeId<Mut<T>>();

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

		IPool* pool = pools[index].GetPool();
		return *static_cast<TPool<Mut<T>>*>(pool);
	}

	template<typename T>
	inline PoolInstance IdContext::CreatePoolInstance() const
	{
		constexpr TypeId componentId = GetTypeId<Mut<T>>();

		auto& self = const_cast<IdContext&>(*this);
		PoolInstance instance{componentId, MakeUnique<TPool<Mut<T>>>(self)};
		return Move(instance);
	}

	template<typename Static>
	inline Static& IdContext::SetStatic()
	{
		OwnPtr& ptr = FindOrAddStaticPtr(statics, GetTypeId<Static>());
		ptr         = MakeOwned<Static>();
		return *ptr.GetUnsafe<Static>();
	}

	template<typename Static>
	inline Static& IdContext::SetStatic(Static&& value)
	{
		OwnPtr& ptr = FindOrAddStaticPtr(statics, GetTypeId<Static>());
		ptr         = MakeOwned<Static>(p::Forward<Static>(value));
		return *ptr.GetUnsafe<Static>();
	}

	template<typename Static>
	inline Static& IdContext::SetStatic(const Static& value)
	{
		OwnPtr& ptr = FindOrAddStaticPtr(statics, GetTypeId<Static>());
		ptr         = MakeOwned<Static>(value);
		return *ptr.GetUnsafe<Static>();
	}

	template<typename Static>
	inline Static& IdContext::GetOrSetStatic()
	{
		bool bAdded = false;
		OwnPtr& ptr = FindOrAddStaticPtr(statics, GetTypeId<Static>(), &bAdded);
		if (bAdded)
		{
			ptr = MakeOwned<Static>();
		}
		return *ptr.GetUnsafe<Static>();
	}

	template<typename Static>
	inline Static& IdContext::GetOrSetStatic(Static&& value)
	{
		bool bAdded = false;
		OwnPtr& ptr = FindOrAddStaticPtr(statics, GetTypeId<Static>(), &bAdded);
		if (bAdded)
		{
			ptr = MakeOwned<Static>(p::Forward<Static>(value));
		}
		return *ptr.GetUnsafe<Static>();
	}

	template<typename Static>
	inline Static& IdContext::GetOrSetStatic(const Static& value)
	{
		bool bAdded = false;
		OwnPtr& ptr = FindOrAddStaticPtr(statics, GetTypeId<Static>(), &bAdded);
		if (bAdded)
		{
			ptr = MakeOwned<Static>(value);
		}
		return *ptr.GetUnsafe<Static>();
	}
#pragma endregion Definitions
}    // namespace p

P_NATIVE(p::Id);

template<>
struct std::formatter<p::Id> : public std::formatter<p::u64>
{
	template<typename FormatContext>
	auto format(p::Id id, FormatContext& ctx) const
	{
		if (id == p::NoId)
		{
			return std::format_to(ctx.out(), "NoId");
		}
		const auto index   = id.GetIndex();
		const auto version = id.GetVersion();
		return std::vformat_to(ctx.out(), "{}:{}", std::make_format_args(index, version));
	}
};
