// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Memory/Block.h"


namespace p
{
	template<typename Type, u32 InlineCapacity>
	struct TInlineArray;


	/** Arena defines the API used on all other arena types */
	class PIPE_API Arena
	{
	public:
		using AllocSignature        = void*(Arena*, sizet size);
		using AllocAlignedSignature = void*(Arena*, sizet size, sizet align);
		using ResizeSignature       = bool(Arena*, void* ptr, sizet ptrSize, sizet size);
		using FreeSignature         = void(Arena*, void* ptr, sizet size);

	private:
		AllocSignature* doAlloc               = nullptr;
		AllocAlignedSignature* doAllocAligned = nullptr;
		ResizeSignature* doRealloc            = nullptr;
		FreeSignature* doFree                 = nullptr;


		template<typename T>
		static consteval bool ImplementsAlloc()
		{
#if defined(__GNUC__)    // GCC won't detect this check as constexpr! :(
			return true;
#else
			return (void* (T::*)(sizet size))(&T::Alloc)
			    != (void* (T::*)(sizet size))(&Arena::Alloc);
#endif
		}
		template<typename T>
		static consteval bool ImplementsAllocAligned()
		{
#if defined(__GNUC__)    // GCC won't detect this check as constexpr! :(
			return true;
#else
			return (void* (T::*)(sizet size, sizet align))(&T::Alloc)
			    != (void* (T::*)(sizet size, sizet align))(&Arena::Alloc);
#endif
		}
		template<typename T>
		static consteval bool ImplementsRealloc()
		{
#if defined(__GNUC__)    // GCC won't detect this check as constexpr! :(
			return true;
#else
			return &T::Realloc != &Arena::Realloc;
#endif
		}
		template<typename T>
		static consteval bool ImplementsFree()
		{
#if defined(__GNUC__)    // GCC won't detect this check as constexpr! :(
			return true;
#else
			return &T::Free != &Arena::Free;
#endif
		}

	protected:

		template<Derived<Arena, false> T>
		void Interface()
		{
			doAlloc = [](Arena* self, sizet size) {
				static_assert(ImplementsAlloc<T>() && "Alloc is not implemented");
				return static_cast<T*>(self)->Alloc(size);
			};
			doAllocAligned = [](Arena* self, sizet size, sizet align) {
				static_assert(ImplementsAllocAligned<T>() && "Alloc (aligned) is not implemented");
				return static_cast<T*>(self)->Alloc(size, align);
			};
			doRealloc = [](Arena* self, void* ptr, sizet ptrSize, sizet size) {
				static_assert(ImplementsRealloc<T>() && "Realloc is not implemented");
				return static_cast<T*>(self)->Realloc(ptr, ptrSize, size);
			};
			doFree = [](Arena* self, void* ptr, sizet size) {
				static_assert(ImplementsFree<T>() && "Free is not implemented");
				return static_cast<T*>(self)->Free(ptr, size);
			};
		}

	public:
		Arena() = default;
		virtual ~Arena() {}
		Arena(const Arena&)            = delete;
		Arena& operator=(const Arena&) = delete;

		Arena(Arena&&)            = default;
		Arena& operator=(Arena&&) = default;


		void* Alloc(sizet size)
		{
			return doAlloc(this, size);
		}
		void* Alloc(sizet size, sizet align)
		{
			return doAllocAligned(this, size, align);
		}
		bool Realloc(void* ptr, sizet ptrSize, sizet size)
		{
			return doRealloc(this, ptr, ptrSize, size);
		}
		void Free(void* ptr, sizet size)
		{
			doFree(this, ptr, size);
		}


		virtual sizet GetUsedMemory() const
		{
			return 0;
		}
		virtual sizet GetAvailableMemory() const
		{
			return 0;
		}
		virtual void GetBlocks(TInlineArray<Memory::Block, 0>& outBlocks) const {}
	};

	class PIPE_API ChildArena : public Arena
	{
	protected:
		Arena* parent = nullptr;

	public:
		ChildArena(Arena* parent);

		Arena& GetParentArena() const
		{
			return *parent;
		}
	};
}    // namespace p
