// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Export.h"


namespace p
{
	namespace Memory
	{
		constexpr sizet B  = 1;
		constexpr sizet KB = 1024;
		constexpr sizet MB = KB * 1024;
		constexpr sizet GB = MB * 1024;
		constexpr sizet TB = GB * 1024;
		constexpr sizet PB = TB * 1024;
	}    // namespace Memory


	PIPE_API void MoveMem(void* dest, void* src, sizet size);
	PIPE_API void CopyMem(void* dest, void* src, sizet size);
	PIPE_API void SwapMem(void* a, void* b, sizet size);
	PIPE_API void SetMem(void* dest, u8 value, sizet size);
	PIPE_API void SetZeroMem(void* dest, sizet size);
	PIPE_API i32 CmpMem(void* a, void* b, sizet size);

	/**
	 * @return the previous pointer of 'ptr' aligned to blockSize
	 */
	void* GetAlignedBlock(void* ptr, const sizet blockSize);

	/**
	 * @return the number of bytes needed for p to be aligned in 'align'
	 */
	PIPE_API sizet GetAlignmentPadding(const void* p, sizet align);

	PIPE_API sizet GetAlignmentPaddingWithHeader(const void* ptr, sizet align, sizet headerSize);


	/**
	 * @brief
	 *
	 * @tparam T
	 * @tparam destroySourceInPlace
	 * @param dest destination memory block
	 * @param src source memory block
	 * @param count
	 */
	template<typename T, bool destroySourceInPlace = false>
	constexpr void CopyItems(T* dest, const T* src, sizet count)
	{
		if constexpr (!std::is_constant_evaluated() && IsTriviallyCopyAssignable<T>)
		{
			CopyMem(dest, src, count * sizeof(T));
		}
		else
		{
			const T* const end = src + count;
			while (src < end)
			{
				*dest = *src;
				if constexpr (destroySourceInPlace)
				{
					src->T::~T();
				}
				++src;
				++dest;
			}
		}
	}

	template<typename T, bool destroySourceInPlace = false>
	constexpr void MoveItems(T* dest, const T* src, sizet count)
	{
		if constexpr (!std::is_constant_evaluated() && IsTriviallyMoveAssignable<T>)
		{
			MoveMem(dest, src, count * sizeof(T));
		}
		else
		{
			const T* const end = src + count;
			while (src < end)
			{
				*dest = Move(*src);
				if constexpr (destroySourceInPlace)
				{
					src->T::~T();
				}
				++src;
				++dest;
			}
		}
	}

	template<typename T, bool destroySourceInPlace = false>
	constexpr void UninitializedCopyItems(T* dest, const T* src, sizet count)
	{
		if constexpr (!std::is_constant_evaluated() && IsTriviallyCopyConstructible<T>)
		{
			CopyMem(dest, src, count * sizeof(T));
		}
		else
		{
			const T* const end = src + count;
			while (src < end)
			{
				new (dest) T(*src);
				if constexpr (destroySourceInPlace)
				{
					src->T::~T();
				}
				++src;
				++dest;
			}
		}
	}

	template<typename T, bool destroySourceInPlace = false>
	constexpr void UninitializedMoveItems(T* dest, const T* src, sizet count)
	{
		if constexpr (!std::is_constant_evaluated() && IsTriviallyMoveConstructible<T>)
		{
			MoveMem(dest, src, count * sizeof(T));
		}
		else
		{
			const T* const end = src + count;
			while (src < end)
			{
				new (dest) T(Move(*src));
				if constexpr (destroySourceInPlace)
				{
					src->T::~T();
				}
				++src;
				++dest;
			}
		}
	}
}    // namespace p
