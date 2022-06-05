// Copyright 2015-2022 Piperift - All rights reserved


#if defined(__cplusplus)
#	include "Memory/Alloc.h"
#	include <new>

void operator delete(void* p) noexcept
{
	pipe::Free(p);
};
void operator delete[](void* p) noexcept
{
	pipe::Free(p);
};

void* operator new(std::size_t n) noexcept(false)
{
	return pipe::Alloc(n);
}
void* operator new[](std::size_t n) noexcept(false)
{
	return pipe::Alloc(n);
}

void* operator new(std::size_t n, const std::nothrow_t& tag) noexcept
{
	(void)(tag);
	return pipe::Alloc(n);
}
void* operator new[](std::size_t n, const std::nothrow_t& tag) noexcept
{
	(void)(tag);
	return pipe::Alloc(n);
}

#	if (__cplusplus >= 201402L || _MSC_VER >= 1916)

void operator delete(void* p, std::size_t n) noexcept
{
	(void)(n);
	pipe::Free(p);
};
void operator delete[](void* p, std::size_t n) noexcept
{
	(void)(n);
	pipe::Free(p);
};

#	endif

#	if (__cplusplus > 201402L || defined(__cpp_aligned_new))

void operator delete(void* p, std::align_val_t align) noexcept
{
	(void)(align);
	pipe::Free(p);
}
void operator delete[](void* p, std::align_val_t align) noexcept
{
	(void)(align);
	pipe::Free(p);
}
void operator delete(void* p, std::size_t n, std::align_val_t align) noexcept
{
	(void)(n);
	(void)(align);
	pipe::Free(p);
};
void operator delete[](void* p, std::size_t n, std::align_val_t align) noexcept
{
	(void)(n);
	(void)(align);
	pipe::Free(p);
};

void* operator new(std::size_t n, std::align_val_t align) noexcept(false)
{
	return pipe::Alloc(n, size_t(align));
}
void* operator new[](std::size_t n, std::align_val_t align) noexcept(false)
{
	return pipe::Alloc(n, size_t(align));
}
void* operator new(std::size_t n, std::align_val_t align, const std::nothrow_t&) noexcept
{
	return pipe::Alloc(n, size_t(align));
}
void* operator new[](std::size_t n, std::align_val_t align, const std::nothrow_t&) noexcept
{
	return pipe::Alloc(n, size_t(align));
}
#	endif
#endif
