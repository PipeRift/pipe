// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Function.h"
#include "Pipe/Core/Platform.h"


namespace p
{
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
		using TResizeSignature = bool (T::*)(void* ptr, sizet ptrSize, sizet size);
		template<typename T>
		using TFreeSignature = void (T::*)(void* ptr, sizet size);

	private:
		TFunction<AllocSignature> doAlloc;
		TFunction<AllocAlignedSignature> doAllocAligned;
		TFunction<ResizeSignature> doResize;
		TFunction<FreeSignature> doFree;


	protected:

		template<typename T, TAllocSignature<T> alloc, TAllocAlignedSignature<T> allocAligned,
		    TResizeSignature<T> resize, TFreeSignature<T> free>
		void SetupInterface()
		{
			doAlloc = [](Arena* self, sizet size) {
				return (static_cast<T*>(self)->*alloc)(size);
			};
			doAllocAligned = [](Arena* self, sizet size, sizet align) {
				return (static_cast<T*>(self)->*allocAligned)(size, align);
			};
			doResize = [](Arena* self, void* ptr, sizet ptrSize, sizet size) {
				return (static_cast<T*>(self)->*resize)(ptr, ptrSize, size);
			};
			doFree = [](Arena* self, void* ptr, sizet size) {
				return (static_cast<T*>(self)->*free)(ptr, size);
			};
		}

	public:
		Arena() = default;
		virtual ~Arena() {}
		Arena(const Arena&) = delete;
		Arena& operator=(const Arena&) = delete;

		Arena(Arena&&) = default;
		Arena& operator=(Arena&&) = default;


		void* Alloc(sizet size)
		{
			return doAlloc(this, size);
		}
		void* Alloc(sizet size, sizet align)
		{
			return doAllocAligned(this, size, align);
		}
		bool Resize(void* ptr, sizet ptrSize, sizet size)
		{
			return doResize(this, ptr, ptrSize, size);
		}
		void Free(void* ptr, sizet size)
		{
			doFree(this, ptr, size);
		}
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
