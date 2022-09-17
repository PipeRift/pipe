// Copyright 2015-2022 Piperift - All rights reserved

// This header should be included in only one source file!
//
// Provides convenient overrides for the new and
// delete operations in C++.

#include "Pipe/Export.h"
#include "Pipe/Memory/Alloc.h"

#include <new>


#define P_DO_OVERRIDE_NEW_DELETE                                                                   \
	void* operator new(std::size_t size) noexcept(false)                                           \
	{                                                                                              \
		return p::HeapAlloc(size);                                                                 \
	}                                                                                              \
	void* operator new[](std::size_t size) noexcept(false)                                         \
	{                                                                                              \
		return p::HeapAlloc(size);                                                                 \
	}                                                                                              \
	void* operator new(std::size_t size, const std::nothrow_t&) noexcept                           \
	{                                                                                              \
		return p::HeapAlloc(size);                                                                 \
	}                                                                                              \
	void* operator new[](std::size_t size, const std::nothrow_t&) noexcept                         \
	{                                                                                              \
		return p::HeapAlloc(size);                                                                 \
	}                                                                                              \
	void* operator new(std::size_t size, std::align_val_t align) noexcept(false)                   \
	{                                                                                              \
		return p::HeapAlloc(size, p::sizet(align));                                                \
	}                                                                                              \
	void* operator new[](std::size_t size, std::align_val_t align) noexcept(false)                 \
	{                                                                                              \
		return p::HeapAlloc(size, p::sizet(align));                                                \
	}                                                                                              \
	void* operator new(std::size_t size, std::align_val_t align, const std::nothrow_t&) noexcept   \
	{                                                                                              \
		return p::HeapAlloc(size, p::sizet(align));                                                \
	}                                                                                              \
	void* operator new[](std::size_t size, std::align_val_t align, const std::nothrow_t&) noexcept \
	{                                                                                              \
		return p::HeapAlloc(size, p::sizet(align));                                                \
	}                                                                                              \
	void operator delete(void* ptr) noexcept                                                       \
	{                                                                                              \
		p::HeapFree(ptr);                                                                          \
	};                                                                                             \
	void operator delete[](void* ptr) noexcept                                                     \
	{                                                                                              \
		p::HeapFree(ptr);                                                                          \
	};                                                                                             \
	void operator delete(void* ptr, const std::nothrow_t&) noexcept                                \
	{                                                                                              \
		p::HeapFree(ptr);                                                                          \
	}                                                                                              \
	void operator delete[](void* ptr, const std::nothrow_t&) noexcept                              \
	{                                                                                              \
		p::HeapFree(ptr);                                                                          \
	}                                                                                              \
	void operator delete(void* ptr, std::size_t) noexcept                                          \
	{                                                                                              \
		p::HeapFree(ptr);                                                                          \
	};                                                                                             \
	void operator delete[](void* ptr, std::size_t) noexcept                                        \
	{                                                                                              \
		p::HeapFree(ptr);                                                                          \
	};                                                                                             \
	void operator delete(void* ptr, std::align_val_t) noexcept                                     \
	{                                                                                              \
		p::HeapFree(ptr);                                                                          \
	}                                                                                              \
	void operator delete[](void* ptr, std::align_val_t) noexcept                                   \
	{                                                                                              \
		p::HeapFree(ptr);                                                                          \
	}                                                                                              \
	void operator delete(void* ptr, std::size_t, std::align_val_t) noexcept                        \
	{                                                                                              \
		p::HeapFree(ptr);                                                                          \
	};                                                                                             \
	void operator delete[](void* ptr, std::size_t, std::align_val_t) noexcept                      \
	{                                                                                              \
		p::HeapFree(ptr);                                                                          \
	};                                                                                             \
	void operator delete(void* ptr, std::align_val_t, const std::nothrow_t&) noexcept              \
	{                                                                                              \
		p::HeapFree(ptr);                                                                          \
	}                                                                                              \
	void operator delete[](void* ptr, std::align_val_t, const std::nothrow_t&) noexcept            \
	{                                                                                              \
		p::HeapFree(ptr);                                                                          \
	}


// If Pipe is linked statically, only Pipe overrides new/delete
#ifdef P_STATIC_DEFINE
#	define P_OVERRIDE_NEW_DELETE
#else
#	define P_OVERRIDE_NEW_DELETE P_DO_OVERRIDE_NEW_DELETE
#endif
