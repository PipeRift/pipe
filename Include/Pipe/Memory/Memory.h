// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Export.h"


namespace p
{
	PIPE_API void MoveMem(void* dest, void* src, sizet size);
	PIPE_API void CopyMem(void* dest, void* src, sizet size);
	PIPE_API void SwapMem(void* a, void* b, sizet size);
	PIPE_API void SetMem(void* dest, u8 value, sizet size);
	PIPE_API void SetZeroMem(void* dest, sizet size);
	PIPE_API i32 CmpMem(void* a, void* b, sizet size);

	/**
	 * @return the number of bytes needed for p to be aligned in 'align'
	 */
	PIPE_API sizet GetAlignmentPadding(const void* p, sizet align);

	PIPE_API sizet GetAlignmentPaddingWithHeader(const void* ptr, sizet align, sizet headerSize);

	template<typename T>
	PIPE_API void MoveConstructItems(T* dest, const T* src, sizet size)
	{
		if constexpr (IsTriviallyCopyable<T>)
		{
			MoveMem(dest, src, size * sizeof(T));
		}
		else
		{
			T* const end = src + size;
			while (src < end)
			{
				new (dest) T(Move(*src));
				src->T::~T();
				++src;
				++dest;
			}
		}
	}
}    // namespace p
