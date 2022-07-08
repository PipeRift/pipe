// Copyright 2015-2022 Piperift - All rights reserved


#if defined(__cplusplus)
#	include "Pipe/Memory/Alloc.h"
#	include <new>

void operator delete(void* ptr) noexcept
{
	p::Free(ptr);
};
void operator delete[](void* ptr) noexcept
{
	p::Free(ptr);
};

void* operator new(std::size_t size) noexcept(false)
{
	return p::Alloc(size);
}
void* operator new[](std::size_t size) noexcept(false)
{
	return p::Alloc(size);
}

void* operator new(std::size_t size, const std::nothrow_t&) noexcept
{
	return p::Alloc(size);
}
void* operator new[](std::size_t size, const std::nothrow_t&) noexcept
{
	return p::Alloc(size);
}

#	if (__cplusplus >= 201402L || _MSC_VER >= 1916)

void operator delete(void* ptr, std::size_t) noexcept
{
	p::Free(ptr);
};
void operator delete[](void* ptr, std::size_t) noexcept
{
	p::Free(ptr);
};

#	endif

#	if (__cplusplus > 201402L || defined(__cpp_aligned_new))

void operator delete(void* ptr, std::align_val_t) noexcept
{
	p::Free(ptr);
}
void operator delete[](void* ptr, std::align_val_t) noexcept
{
	p::Free(ptr);
}
void operator delete(void* ptr, std::size_t, std::align_val_t) noexcept
{
	p::Free(ptr);
};
void operator delete[](void* ptr, std::size_t, std::align_val_t) noexcept
{
	p::Free(ptr);
};

void* operator new(std::size_t size, std::align_val_t align) noexcept(false)
{
	return p::Alloc(size, p::sizet(align));
}
void* operator new[](std::size_t size, std::align_val_t align) noexcept(false)
{
	return p::Alloc(size, p::sizet(align));
}
void* operator new(std::size_t size, std::align_val_t align, const std::nothrow_t&) noexcept
{
	return p::Alloc(size, p::sizet(align));
}
void* operator new[](std::size_t size, std::align_val_t align, const std::nothrow_t&) noexcept
{
	return p::Alloc(size, p::sizet(align));
}
#	endif
#endif
