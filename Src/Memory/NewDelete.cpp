// Copyright 2015-2021 Piperift - All rights reserved

/*
#if defined(__cplusplus)
#	include <rpmalloc/rpmalloc.h>
#	include <Tracy.hpp>
#	include <new>

void operator delete(void* p) noexcept
{
	TracyFree(p);
	rpfree(p);
};
void operator delete[](void* p) noexcept
{
	TracyFree(p);
	rpfree(p);
};

void* operator new(std::size_t n) noexcept(false)
{
	void* const p = rpmalloc(n);
	TracyAlloc(p, n);
	return p;
}
void* operator new[](std::size_t n) noexcept(false)
{
	void* const p = rpmalloc(n);
	TracyAlloc(p, n);
	return p;
}

void* operator new(std::size_t n, const std::nothrow_t& tag) noexcept
{
	(void) (tag);
	void* const p = rpmalloc(n);
	TracyAlloc(p, n);
	return p;
}
void* operator new[](std::size_t n, const std::nothrow_t& tag) noexcept
{
	(void) (tag);
	void* const p = rpmalloc(n);
	TracyAlloc(p, n);
	return p;
}

#	if (__cplusplus >= 201402L || _MSC_VER >= 1916)

void operator delete(void* p, std::size_t n) noexcept
{
	(void) (n);
	TracyFree(p);
	rpfree(p);
};
void operator delete[](void* p, std::size_t n) noexcept
{
	(void) (n);
	TracyFree(p);
	rpfree(p);
};

#	endif

#	if (__cplusplus > 201402L || defined(__cpp_aligned_new))

void operator delete(void* p, std::align_val_t align) noexcept
{
	(void) (align);
	TracyFree(p);
	rpfree(p);
}
void operator delete[](void* p, std::align_val_t align) noexcept
{
	(void) (align);
	TracyFree(p);
	rpfree(p);
}
void operator delete(void* p, std::size_t n, std::align_val_t align) noexcept
{
	(void) (n);
	(void) (align);
	TracyFree(p);
	rpfree(p);
};
void operator delete[](void* p, std::size_t n, std::align_val_t align) noexcept
{
	(void) (n);
	(void) (align);
	TracyFree(p);
	rpfree(p);
};

void* operator new(std::size_t n, std::align_val_t align) noexcept(false)
{
	void* const p = rpaligned_alloc(size_t(align), n);
	TracyAlloc(p, n);
	return p;
}
void* operator new[](std::size_t n, std::align_val_t align) noexcept(false)
{
	void* const p = rpaligned_alloc(size_t(align), n);
	TracyAlloc(p, n);
	return p;
}
void* operator new(std::size_t n, std::align_val_t align, const std::nothrow_t&) noexcept
{
	void* const p = rpaligned_alloc(size_t(align), n);
	TracyAlloc(p, n);
	return p;
}
void* operator new[](std::size_t n, std::align_val_t align, const std::nothrow_t&) noexcept
{
	void* const p = rpaligned_alloc(size_t(align), n);
	TracyAlloc(p, n);
	return p;
}
#	endif
#endif
*/