// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#if PLATFORM_WINDOWS
#	include "Platform/Windows/WindowsPlatform.h"
#elif PLATFORM_LINUX
#	include "Platform/Linux/LinuxPlatform.h"
#elif PLATFORM_MACOS
#	include "Platform/Mac/MacPlatform.h"
#else
#	error Unknown platform
#endif


namespace Rift
{
	//------------------------------------------------------------------
	// Transfer the platform types to global types
	//------------------------------------------------------------------

	//~ Unsigned base types.
	/// An 8-bit unsigned integer.
	typedef PlatformTypes::uint8 u8;
	/// A 16-bit unsigned integer.
	using u16 = PlatformTypes::uint16;
	/// A 32-bit unsigned integer.
	using u32 = PlatformTypes::uint32;
	/// A 64-bit unsigned integer.
	using u64 = PlatformTypes::uint64;

	//~ Signed base types.
	/// An 8-bit signed integer.
	using i8 = PlatformTypes::int8;
	/// A 16-bit signed integer.
	using i16 = PlatformTypes::int16;
	/// A 32-bit signed integer.
	using i32 = PlatformTypes::int32;
	/// A 64-bit signed integer.
	using i64 = PlatformTypes::int64;

	//~ Character types.
	/// An ANSI character. Normally a signed type.
	using ANSICHAR = PlatformTypes::ANSICHAR;
	/// A wide character. Normally a signed type.
	using WIDECHAR = PlatformTypes::WIDECHAR;
	/// Either ANSICHAR or WIDECHAR, depending on whether the platform supports wide
	/// characters or the requirements of the licensee.
	using TCHAR = PlatformTypes::TCHAR;
	/// An 8-bit character containing a UTF8 (Unicode, 8-bit, variable-width) code
	/// unit.
	using UTF8CHAR = PlatformTypes::CHAR8;
	/// A 16-bit character containing a UCS2 (Unicode, 16-bit, fixed-width) code
	/// unit, used for compatibility with 'Windows TCHAR' across multiple platforms.
	using UCS2CHAR = PlatformTypes::CHAR16;
	/// A 16-bit character containing a UTF16 (Unicode, 16-bit, variable-width) code
	/// unit.
	using UTF16CHAR = PlatformTypes::CHAR16;
	/// A 32-bit character containing a UTF32 (Unicode, 32-bit, fixed-width) code
	/// unit.
	using UTF32CHAR = PlatformTypes::CHAR32;

	/// An unsigned integer the same size as a pointer
	using uPtr = PlatformTypes::uPtr;
	/// A signed integer the same size as a pointer
	using iPtr = PlatformTypes::iPtr;
	/// An unsigned integer the same size as a pointer, the same as UPTRINT
	using sizet = PlatformTypes::sizet;
	/// An integer the same size as a pointer, the same as PTRINT
	using ssizet = PlatformTypes::ssizet;

	/// The type of the NULL constant.
	using TYPE_OF_NULL = PlatformTypes::TYPE_OF_NULL;
	/// The type of the C++ nullptr keyword.
	using TYPE_OF_NULLPTR = PlatformTypes::TYPE_OF_NULLPTR;
}    // namespace Rift

#if !defined(TX)
#	if PLATFORM_TCHAR_IS_1_BYTE
#		define TEXT_PASTE(x) x
#	elif PLATFORM_TCHAR_IS_CHAR16
#		define TEXT_PASTE(x) u##x
#	elif PLATFORM_TCHAR_IS_WCHAR
#		define TEXT_PASTE(x) L##x
#	else
#		define TEXT_PASTE(x) x
#	endif
#	define TX(x) TEXT_PASTE(x)
#endif

#undef UNIQUE_FUNCTION_ID
#if defined(_MSC_VER)
#	define UNIQUE_FUNCTION_ID __FUNCSIG__
#else
#	if defined(__GNUG__)
#		define UNIQUE_FUNCTION_ID __PRETTY_FUNCTION__
#	endif
#endif
