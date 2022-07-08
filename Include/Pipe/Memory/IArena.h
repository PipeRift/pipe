// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Function.h"


namespace p
{
	/** IArena defines the API used on all other arena types */
	class PIPE_API IArena
	{
	public:
		using AllocSignature        = void*(sizet size);
		using AllocAlignedSignature = void*(sizet size, sizet align);
		using ResizeSignature       = bool(void* ptr, sizet ptrSize, sizet size);
		using FreeSignature         = void(void* ptr, sizet size);
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
		template<typename T>
		void SetupInterface(TAllocSignature<T> doAllocIn,
		    TAllocAlignedSignature<T> doAllocAlignedIn, TResizeSignature<T> doResizeIn,
		    TFreeSignature<T> doFreeIn)
		{
			doAlloc = [this, doAllocIn](sizet size) {
				return (static_cast<T*>(this)->*doAllocIn)(size);
			};
			doAllocAligned = [this, doAllocAlignedIn](sizet size, sizet align) {
				return (static_cast<T*>(this)->*doAllocAlignedIn)(size, align);
			};
			doResize = [this, doResizeIn](void* ptr, sizet ptrSize, sizet size) {
				return (static_cast<T*>(this)->*doResizeIn)(ptr, ptrSize, size);
			};
			doFree = [this, doFreeIn](void* ptr, sizet size) {
				(static_cast<T*>(this)->*doFreeIn)(ptr, size);
			};
		}

	public:
		virtual ~IArena() {}

		void* Alloc(sizet size)
		{
			return doAlloc(size);
		}
		void* Alloc(sizet size, sizet align)
		{
			return doAllocAligned(size, align);
		}
		bool Resize(void* ptr, sizet ptrSize, sizet size)
		{
			return doResize(ptr, ptrSize, size);
		}
		void Free(void* ptr, sizet size)
		{
			doFree(ptr, size);
		}
	};
}    // namespace p
