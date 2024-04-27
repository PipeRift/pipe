// Copyright 2015-2024 Piperift - All rights reserved

// This header should be included in only one source file!
//
// Provides convenient overrides for the new and
// delete operations in C++.

#pragma once


#if defined(__cplusplus)
	#include "Pipe/Core/Platform.h"
	#include "Pipe/Memory/Alloc.h"
	#include <new>


	#if defined(_MSC_VER) && defined(_Ret_notnull_) && defined(_Post_writable_byte_size_)
	    // stay consistent with VCRT definitions
		#define P_DECL_NEW(n) P_NODISCARD P_RESTRICT _Ret_notnull_ _Post_writable_byte_size_(n)
		#define P_DECL_NEW_NOTHROW(n)                                        \
			P_NODISCARD P_RESTRICT _Ret_maybenull_ _Success_(return != NULL) \
			    _Post_writable_byte_size_(n)
	#else
		#define P_DECL_NEW(n) P_NODISCARD P_RESTRICT
		#define P_DECL_NEW_NOTHROW(n) P_NODISCARD P_RESTRICT
	#endif


// clang-format off
P_DECL_NEW(n) void* operator new(std::size_t n) noexcept(false) { return p::HeapAlloc(n); }
P_DECL_NEW(n) void* operator new[](std::size_t n) noexcept(false) { return p::HeapAlloc(n); }

P_DECL_NEW_NOTHROW(n) void* operator new  (std::size_t n, const std::nothrow_t& tag) noexcept { (void)(tag); return p::HeapAlloc(n); }
P_DECL_NEW_NOTHROW(n) void* operator new[](std::size_t n, const std::nothrow_t& tag) noexcept { (void)(tag); return p::HeapAlloc(n); }

void operator delete(void* p) noexcept              { p::HeapFree(p); };
void operator delete[](void* p) noexcept            { p::HeapFree(p); };

void operator delete  (void* p, const std::nothrow_t&) noexcept { p::HeapFree(p); }
void operator delete[](void* p, const std::nothrow_t&) noexcept { p::HeapFree(p); }

#	if (__cplusplus >= 201402L || _MSC_VER >= 1916)
void operator delete  (void* p, std::size_t n) noexcept { p::HeapFree(p); };
void operator delete[](void* p, std::size_t n) noexcept { p::HeapFree(p); };
#	endif

#	if (__cplusplus > 201402L || defined(__cpp_aligned_new))
void* operator new  (std::size_t n, std::align_val_t al) noexcept(false) { return p::HeapAlloc(n, p::sizet(al)); }
void* operator new[](std::size_t n, std::align_val_t al) noexcept(false) { return p::HeapAlloc(n, p::sizet(al)); }
void* operator new  (std::size_t n, std::align_val_t al, const std::nothrow_t&) noexcept { return p::HeapAlloc(n, p::sizet(al)); }
void* operator new[](std::size_t n, std::align_val_t al, const std::nothrow_t&) noexcept { return p::HeapAlloc(n, p::sizet(al)); }

void operator delete  (void* p, std::align_val_t) noexcept { p::HeapFree(p); }
void operator delete[](void* p, std::align_val_t) noexcept { p::HeapFree(p); }
void operator delete  (void* p, std::size_t, std::align_val_t ) noexcept { p::HeapFree(p); };
void operator delete[](void* p, std::size_t, std::align_val_t ) noexcept { p::HeapFree(p); };
void operator delete  (void* p, std::align_val_t, const std::nothrow_t&) noexcept { p::HeapFree(p); }
void operator delete[](void* p, std::align_val_t, const std::nothrow_t&) noexcept { p::HeapFree(p); }
#	endif
// clang-format on

	#define P_DO_OVERRIDE_NEW_DELETE


    // If Pipe is linked statically, only Pipe overrides new/delete
	#ifdef PIPE_STATIC_DEFINE
		#define P_OVERRIDE_NEW_DELETE
	#else
		#define P_OVERRIDE_NEW_DELETE P_DO_OVERRIDE_NEW_DELETE
	#endif
#endif
