// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Memory/Arena.h"
#include "PipeArrays.h"
#include "PipePlatform.h"

#include <memory>



namespace p
{
	/** List of instances stored in pages of an specific size (N items per page).
	 * A "buffer" because instances are treated as memory and removing elements does not shift
	 * elements behind. Growing or shrinking expands the pages of instances but DOES NOT handle
	 * constructors and destructors. This should be done by the user (Insert, RemoveAt, Swap, etc).
	 */
	template<typename Type, i32 PageSize>
	struct TPageBuffer
	{
		static_assert(!IsVoid<Type>, "PageBuffer's type can't be void");

		template<typename OtherType, i32 OtherPageSize>
		friend struct TPageBuffer;

		using ItemType                = Type;
		static constexpr i32 pageSize = PageSize;

	protected:
		TArray<Type*> pages;
		Arena* arena = nullptr;

	public:
		TFunction<void(sizet index, Type* page, i32 size)> onPageAllocated;


	public:
		TPageBuffer(Arena& arena) : pages(arena), arena{&arena} {};
		~TPageBuffer()
		{
			Shrink(0);
		}

		TPageBuffer(TPageBuffer&& other) noexcept
		{
			MoveFrom(Move(other));
		}
		TPageBuffer& operator=(TPageBuffer&& other) noexcept
		{
			MoveFrom(Move(other));
			return *this;
		}

		template<typename... Args>
		Type* Insert(sizet index, Args&&... args)
		{
			P_CheckMsg(index < Capacity(), "Out of bounds index");
			Type* const instance = AssurePage(index) + GetOffset(index);
			std::construct_at(instance, p::Forward<Args>(args)...);
			return instance;
		}

		void RemoveAtPage(i32 page, i32 offset)
		{
			std::destroy_at(pages[page] + offset);
		}
		void RemoveAt(sizet index)
		{
			RemoveAtPage(GetPage(index), GetOffset(index));
		}

		void Swap(sizet aIndex, sizet bIndex)
		{
			p::Swap((*this)[aIndex], (*this)[bIndex]);
		}

		// Allocate enough memory to allocate N elements. Reserve doesn't shrink.
		void Reserve(sizet size)
		{
			if (size > Capacity()) [[unlikely]]
			{
				sizet pagesNeeded = GetPage(size - 1) + 1;

				pages.Resize(pagesNeeded, nullptr);
			}
		}

		// Shrinks allocated memory to size. Does not destroy instances!!
		void Shrink(sizet size)
		{
			const auto from = (size + pageSize - 1u) / pageSize;
			for (auto pos = from; pos < pages.Size(); ++pos)
			{
				Type* const page = pages[pos];
				if (page)
				{
					p::Free(*arena, page, pageSize);
				}
			}
			pages.Resize(from);
		}

		void Clear()
		{
			Shrink(0);
		}

		i32 GetPagesSize() const
		{
			return pages.Size();
		}

		sizet Capacity() const
		{
			return pages.Size() * pageSize;
		}

		Type* At(sizet index)
		{
			if (Type* page = pages[GetPage(index)])
			{
				return page + GetOffset(index);
			}
			return nullptr;
		}
		const Type* At(sizet index) const
		{
			const i32 pageIndex = GetPage(index);
			if (pageIndex < pages.Size() && pages[pageIndex])
			{
				return pages[pageIndex] + GetOffset(index);
			}
			return nullptr;
		}
		Type& operator[](sizet index)
		{
			return pages[GetPage(index)][GetOffset(index)];
		}
		const Type& operator[](sizet index) const
		{
			return pages[GetPage(index)][GetOffset(index)];
		}

		Type* AssurePage(sizet index)
		{
			const i32 pageIndex = GetPage(index);
			Type*& page         = pages[pageIndex];
			if (!page) [[unlikely]]
			{
				page = p::Alloc<Type>(*arena, pageSize);
				if (onPageAllocated)
				{
					onPageAllocated(pageIndex, page, pageSize);
				}
			}
			return page;
		}

		Type* FindPage(sizet index) const
		{
			return pages[GetPage(index)];
		}

		const TArray<Type*>& GetPages() const
		{
			return pages;
		}
		static constexpr i32 GetPage(sizet index)
		{
			return i32(index / pageSize);
		}
		static constexpr i32 GetOffset(sizet index)
		{
			return i32(index & (pageSize - 1));
		}

	private:

		void MoveFrom(TPageBuffer&& other)
		{
			Clear();
			pages           = Exchange(other.pages, {});
			arena           = Exchange(other.arena, {});
			onPageAllocated = Exchange(other.onPageAllocated, {});
		}
	};
}    // namespace p
