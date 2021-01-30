// Copyright 2015-2021 Piperift - All rights reserved

#if defined(__cplusplus)
#	include <mimalloc.h>
#	include <Tracy.hpp>
#	include <new>

struct StaticRegisterMimallocOutput
{
	StaticRegisterMimallocOutput()
	{
		mi_register_output(nullptr, nullptr);
	}
} staticRegisterMimallocOutput;


void operator delete(void* p) noexcept
{
	TracyFree(p);
	mi_free(p);
};
void operator delete[](void* p) noexcept
{
	TracyFree(p);
	mi_free(p);
};

void* operator new(std::size_t n) noexcept(false)
{
	void* const p = mi_new(n);
	TracyAlloc(p, n);
	return p;
}
void* operator new[](std::size_t n) noexcept(false)
{
	void* const p = mi_new(n);
	TracyAlloc(p, n);
	return p;
}

void* operator new(std::size_t n, const std::nothrow_t& tag) noexcept
{
	(void) (tag);
	void* const p = mi_new_nothrow(n);
	TracyAlloc(p, n);
	return p;
}
void* operator new[](std::size_t n, const std::nothrow_t& tag) noexcept
{
	(void) (tag);
	void* const p = mi_new_nothrow(n);
	TracyAlloc(p, n);
	return p;
}

#	if (__cplusplus >= 201402L || _MSC_VER >= 1916)
void operator delete(void* p, std::size_t n) noexcept
{
	TracyFree(p);
	mi_free_size(p, n);
};
void operator delete[](void* p, std::size_t n) noexcept
{
	TracyFree(p);
	mi_free_size(p, n);
};
#	endif

#	if (__cplusplus > 201402L || defined(__cpp_aligned_new))
void operator delete(void* p, std::align_val_t al) noexcept
{
	TracyFree(p);
	mi_free_aligned(p, static_cast<size_t>(al));
}
void operator delete[](void* p, std::align_val_t al) noexcept
{
	TracyFree(p);
	mi_free_aligned(p, static_cast<size_t>(al));
}
void operator delete(void* p, std::size_t n, std::align_val_t al) noexcept
{
	TracyFree(p);
	mi_free_size_aligned(p, n, static_cast<size_t>(al));
};
void operator delete[](void* p, std::size_t n, std::align_val_t al) noexcept
{
	TracyFree(p);
	mi_free_size_aligned(p, n, static_cast<size_t>(al));
};

void* operator new(std::size_t n, std::align_val_t al) noexcept(false)
{
	void* const p = mi_new_aligned(n, static_cast<size_t>(al));
	TracyAlloc(p, n);
	return p;
}
void* operator new[](std::size_t n, std::align_val_t al) noexcept(false)
{
	void* const p = mi_new_aligned(n, static_cast<size_t>(al));
	TracyAlloc(p, n);
	return p;
}
void* operator new(std::size_t n, std::align_val_t al, const std::nothrow_t&) noexcept
{
	void* const p = mi_new_aligned_nothrow(n, static_cast<size_t>(al));
	TracyAlloc(p, n);
	return p;
}
void* operator new[](std::size_t n, std::align_val_t al, const std::nothrow_t&) noexcept
{
	void* const p = mi_new_aligned_nothrow(n, static_cast<size_t>(al));
	TracyAlloc(p, n);
	return p;
}
#	endif
#endif
