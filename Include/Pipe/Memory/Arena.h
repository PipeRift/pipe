// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Function.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Memory/Block.h"


namespace p
{
	class ArenaAllocator;
	namespace core
	{
		template<typename Type>
		struct TArray;
	}
	using namespace core;


	/** Arena defines the API used on all other arena types */
	class PIPE_API Arena
	{
	public:
		using AllocSignature        = void*(Arena*, sizet size);
		using AllocAlignedSignature = void*(Arena*, sizet size, sizet align);
		using ResizeSignature       = bool(Arena*, void* ptr, sizet ptrSize, sizet size);
		using FreeSignature         = void(Arena*, void* ptr, sizet size);
		template<typename T>
		using TAllocSignature = void* (T::*)(sizet size);
		template<typename T>
		using TAllocAlignedSignature = void* (T::*)(sizet size, sizet align);
		template<typename T>
		using TReallocSignature = bool (T::*)(void* ptr, sizet ptrSize, sizet size);
		template<typename T>
		using TFreeSignature = void (T::*)(void* ptr, sizet size);

	private:
		TFunction<AllocSignature> doAlloc;
		TFunction<AllocAlignedSignature> doAllocAligned;
		TFunction<ResizeSignature> doRealloc;
		TFunction<FreeSignature> doFree;


	protected:

		template<typename T, TAllocSignature<T> alloc, TAllocAlignedSignature<T> allocAligned,
		    TReallocSignature<T> resize, TFreeSignature<T> free>
		inline void Interface()
		{
			doAlloc = [](Arena* self, sizet size) {
				return (static_cast<T*>(self)->*alloc)(size);
			};
			doAllocAligned = [](Arena* self, sizet size, sizet align) {
				return (static_cast<T*>(self)->*allocAligned)(size, align);
			};
			doRealloc = [](Arena* self, void* ptr, sizet ptrSize, sizet size) {
				return (static_cast<T*>(self)->*resize)(ptr, ptrSize, size);
			};
			doFree = [](Arena* self, void* ptr, sizet size) {
				return (static_cast<T*>(self)->*free)(ptr, size);
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
		virtual void GetBlocks(TArray<Memory::Block>& outBlocks) const {}
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
