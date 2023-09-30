// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Export.h"


namespace p
{
	namespace Internal
	{
		template<typename T>
		struct TIsCharacterByteOrBool : std::false_type
		{};
		template<>
		struct TIsCharacterByteOrBool<char> : std::true_type
		{};    // chars are characters
		template<>
		struct TIsCharacterByteOrBool<signed char> : std::true_type
		{};    // signed chars are also characters
		template<>
		struct TIsCharacterByteOrBool<unsigned char> : std::true_type
		{};    // unsigned chars are also characters
		template<>
		struct TIsCharacterByteOrBool<char8_t> : std::true_type
		{};
		template<>
		struct TIsCharacterByteOrBool<std::byte> : std::true_type
		{};
		template<>
		struct TIsCharacterByteOrBool<bool> : std::true_type
		{};

		template<typename T>
		struct TIsMemsetZeroConstructible
		{
			static constexpr bool value =
			    IsScalar<T> && TIsCharacterByteOrBool<T>::value && !IsVolatile<T>;
		};

		template<typename T>
		struct TIsMemsetValueConstructible
		{
			static constexpr bool value = IsScalar<T> && !IsVolatile<T> && !IsMemberPointer<T>;
		};

		template<typename T>
		bool AreBitsZero(const T& value) requires(IsScalar<T> && !IsMemberPointer<T>)
		{
			if constexpr (IsSame<T, std::nullptr_t>)
			{
				return true;
			}
			else
			{
				constexpr T zero{};
				return CmpMem(&value, &zero, sizeof(T)) == 0;
			}
		}
	}    // namespace Internal

	namespace Memory
	{
		// Kilobyte, Megabyte, Gigabyte...
		constexpr sizet B  = 1;
		constexpr sizet KB = 1024;
		constexpr sizet MB = KB * 1024;
		constexpr sizet GB = MB * 1024;
		constexpr sizet TB = GB * 1024;
		constexpr sizet PB = TB * 1024;

		// Kilobit, Megabit, Gigabit...
		constexpr sizet Kb = KB / 8;
		constexpr sizet Mb = MB / 8;
		constexpr sizet Gb = GB / 8;
		constexpr sizet Tb = TB / 8;
		constexpr sizet Pb = PB / 8;
	}    // namespace Memory


	PIPE_API void MoveMem(void* dest, void* src, sizet size);
	PIPE_API void CopyMem(void* dest, const void* src, sizet size);
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


	/** Constructs a number of contiguous items with the default constructor */
	template<typename T>
	constexpr void ConstructItems(T* data, sizet count)
	{
		if (!std::is_constant_evaluated() && Internal::TIsMemsetZeroConstructible<T>::value)
		{
			SetMem(data, 0, count * sizeof(T));
		}
		else
		{
			const T* const end = data + count;
			while (data < end)
			{
				new (data) T();
				++data;
			}
		}
	}

	/** Constructs a number of contiguous items by copying from a single source item */
	template<typename T>
	constexpr void ConstructItems(T* data, sizet count, const T& value)
	{
		if (!std::is_constant_evaluated())
		{
			if constexpr (Internal::TIsMemsetValueConstructible<T>::value)
			{
				SetMem(data, static_cast<unsigned char>(value), count * sizeof(T));
				return;
			}
			else if constexpr (Internal::TIsMemsetZeroConstructible<T>::value)
			{
				if (Internal::AreBitsZero(value))
				{
					SetMem(data, 0, count * sizeof(T));
					return;
				}
			}
		}

		const T* const end = data + count;
		while (data < end)
		{
			new (data) T(value);
			++data;
		}
	}

	/** Constructs a number of contiguous items by copying from source items */
	template<typename T, bool destroySourceInPlace = false>
	constexpr void CopyConstructItems(T* data, sizet count, const T* values)
	{
		if (!std::is_constant_evaluated() && IsTriviallyCopyConstructible<T>)
		{
			CopyMem(data, values, count * sizeof(T));
		}
		else
		{
			const T* const end = values + count;
			while (values < end)
			{
				new (data) T(*values);
				if constexpr (destroySourceInPlace)
				{
					values->T::~T();
				}
				++values;
				++data;
			}
		}
	}

	template<typename T, bool destroySourceInPlace = false>
	constexpr void MoveConstructItems(T* data, sizet count, T* values)
	{
		if (!std::is_constant_evaluated() && IsTriviallyMoveConstructible<T>)
		{
			MoveMem(data, values, count * sizeof(T));
		}
		else
		{
			const T* const end = values + count;
			while (values < end)
			{
				new (data) T(p::Forward<T>(*values));
				if constexpr (destroySourceInPlace)
				{
					values->T::~T();
				}
				++values;
				++data;
			}
		}
	}

	template<typename T, bool destroySourceInPlace = false>
	constexpr void MoveOrCopyConstructItems(T* data, sizet count, T* values)
	{
		if constexpr (IsMoveConstructible<T> || !IsCopyConstructible<T>)
		{
			MoveConstructItems<T, destroySourceInPlace>(data, count, values);
		}
		else
		{
			CopyConstructItems<T, destroySourceInPlace>(data, count, values);
		}
	}

	template<typename T, bool destroySourceInPlace = false>
	constexpr void CopyItems(T* dest, sizet count, const T* src)
	{
		if (!std::is_constant_evaluated() && IsTriviallyCopyAssignable<T>)
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
	constexpr void MoveItems(T* dest, sizet count, T* source)
	{
		if (!std::is_constant_evaluated() && IsTriviallyMoveAssignable<T>)
		{
			MoveMem(dest, source, count * sizeof(T));
		}
		else
		{
			const T* const end = source + count;
			while (source < end)
			{
				*dest = p::Forward<T>(*source);
				if constexpr (destroySourceInPlace)
				{
					source->T::~T();
				}
				++source;
				++dest;
			}
		}
	}

	// Move items [source, source + count) backwards to [..., dest)
	template<typename T>
	void MoveItemsBackwards(T* dest, sizet count, T* source)
	{
		if (!std::is_constant_evaluated() && IsTriviallyMoveAssignable<T>)
		{
			// MoveMem (or std::memmove) is safe in overlapping memory
			p::MoveMem(dest, source, count * sizeof(T));
		}
		else
		{
			T* lastDest   = dest + count;
			T* lastSource = source + count;
			while (lastSource != source)
			{
				*--lastDest = p::Forward<T>(*--lastSource);
			}
		}
	}

	template<typename T>
	constexpr void DestroyItems(T* data, sizet count)
	{
		if (!std::is_constant_evaluated() && IsTriviallyDestructible<T>)
		{
			// Do nothing. No destruction needed.
		}
		else
		{
			const T* const end = data + count;
			while (data < end)
			{
				data->T::~T();
				++data;
			}
		}
	}
}    // namespace p
