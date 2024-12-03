// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Memory/Block.h"
#include "Pipe/Memory/Memory.h"


namespace p
{
#pragma region Allocation
	class Arena;
	class HeapArena;

	void InitializeMemory();

	// Native allocation functions
	PIPE_API void* HeapAlloc(sizet size);
	PIPE_API void* HeapAlloc(sizet size, sizet align);
	PIPE_API void* HeapRealloc(void* ptr, sizet size);
	PIPE_API void HeapFree(void* ptr);


	PIPE_API HeapArena& GetHeapArena();
	PIPE_API Arena& GetCurrentArena();
	PIPE_API void SetCurrentArena(Arena& arena);

	// Arena allocation functions (Find current arena)
	PIPE_API void* Alloc(sizet size);
	PIPE_API void* Alloc(sizet size, sizet align);
	PIPE_API bool Realloc(void* ptr, sizet ptrSize, sizet size);
	PIPE_API void Free(void* ptr, sizet size);


	// Templated arena allocation functions:

	template<typename T, Derived<Arena> ArenaT>
	T* Alloc(ArenaT& arena, sizet count = 1) requires(!IsVoid<T>)
	{
		return static_cast<T*>(arena.Alloc(sizeof(T) * count, alignof(T)));
	}
	template<typename T, Derived<Arena> ArenaT>
	T* Alloc(ArenaT& arena, sizet count, sizet align) requires(!IsVoid<T>)
	{
		return static_cast<T*>(arena.Alloc(sizeof(T) * count, align));
	}
	template<typename T, Derived<Arena> ArenaT>
	bool Realloc(ArenaT& arena, T* ptr, sizet ptrCount, sizet newCount) requires(!IsVoid<T>)
	{
		return arena.Realloc(ptr, sizeof(T) * ptrCount, sizeof(T) * newCount);
	}
	template<typename T, Derived<Arena> ArenaT>
	void Free(ArenaT& arena, T* ptr, u32 count = 1) requires(!IsVoid<T>)
	{
		arena.Free(static_cast<void*>(ptr), sizeof(T) * count);
	}
#pragma endregion Allocation


#pragma region Arena
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

		virtual sizet GetAvailableMemory() const
		{
			return 0;
		}
		virtual void GetBlocks(TInlineArray<Memory::Block, 0>& outBlocks) const {}

		virtual const struct MemoryStats* GetStats() const
		{
			return nullptr;
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
#pragma endregion Arena
}    // namespace p
