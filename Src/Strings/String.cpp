// Copyright 2015-2021 Piperift - All rights reserved

#include "Strings/String.h"

#include "Math/Math.h"
#include "Misc/Char.h"


namespace Rift
{
	void CString::ToSentenceCase(const String& str, String& result)
	{
		if (!str.empty())
		{
			// static const std::regex wordToCapital("\b[a-z]");
			static const Regex spaceCamelCase(TX("([a-zA-Z])(?=[A-Z0-9])"));

			result = String{std::regex_replace(str.c_str(), spaceCamelCase, TX("$& ")).c_str()};
			result[0] = (char) ::toupper(result[0]);
		}
		else
		{
			result = TX("");
		}
	}

	String CString::BackSubstr(const String& str, i32 size)
	{
		return str.substr(Math::Max(sizet(0u), sizet(str.size() - size)), size);
	}

	bool CString::IsNumeric(const TCHAR* Str)
	{
		if (*Str == TX('-') || *Str == TX('+'))
		{
			Str++;
		}

		bool bHasDot = false;
		while (*Str != TX('\0'))
		{
			if (*Str == TX('.'))
			{
				if (bHasDot)
				{
					return false;
				}
				bHasDot = true;
			}
			else if (!FChar::IsDigit(*Str))
			{
				return false;
			}

			++Str;
		}

		return true;
	}

	String CString::ParseMemorySize(sizet size)
	{
		if (size <= 0)
		{
			return "0B";
		}

		static const char* sizes[]{"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

		const double scaleD = Math::Log(double(size), 1024.l);
		const u32 scale = u32(Math::FloorToI64(scaleD));
		const sizet finalSize = size / Math::Pow(1024, scale) * (1 / size);

		return CString::Format("{}{}", finalSize, sizes[scale]);
	}

	sizet CString::GetStringHash(const TCHAR* str)
	{
		// FNV String hash
		// Use offset and prime based on the architecture (64bit or 32bit)
		// http://www.isthe.com/chongo/tech/comp/fnv/index.html
		static constexpr bool bIs32Bit     = sizeof(sizet) < 64;
		static constexpr sizet offsetBasis = bIs32Bit ? 2166136261U : 14695981039346656037U;
		static constexpr sizet fnvPrime    = bIs32Bit ? 16777619 : 1099511628211;

		sizet c, result = offsetBasis;
		while ((c = *str++) != 0)
		{
			result = (result * fnvPrime) ^ c;
		}
		return result;
	}
}	 // namespace Rift
