
// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Broadcast.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/Span.h"
#include "Pipe/ECS/PageBuffer.h"
#include "Pipe/Memory/UniquePtr.h"
#include "Pipe/PipeECS.h"


namespace p::ecs
{
	struct Context;


	enum class DeletionPolicy : u8
	{
		Swap,
		InPlace
	};


	struct PIPE_API PoolIterator final
	{
		using difference_type   = typename IdTraits<Id>::Difference;
		using value_type        = ecs::Id;
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

		Context* context = nullptr;
		TBroadcast<Context&, TSpan<const Id>> onAdd;
		TBroadcast<Context&, TSpan<const Id>> onRemove;


		BasePool(Context& ast, DeletionPolicy deletionPolicy, Arena& arena = GetCurrentArena())
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
			return GetIndexFromId(ecs::GetIndex(id));
		}
		Id GetIdFromIndex(i32 index) const
		{
			return index < idList.Size() ? idList[index] : ecs::NoId;
		}

		virtual void SetOwnerContext(Context& destination)
		{
			context = &destination;
		}
		virtual TUniquePtr<BasePool> Clone() = 0;

		Context& GetContext() const
		{
			return *context;
		}

		bool Has(Id id) const
		{
			const i32* const index = idIndices.At(ecs::GetIndex(id));
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

		TBroadcast<Context&, TSpan<const Id>>& OnAdd()
		{
			return onAdd;
		}

		TBroadcast<Context&, TSpan<const Id>>& OnRemove()
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
}    // namespace p::ecs
