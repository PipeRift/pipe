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

			result    = String{std::regex_replace(str.c_str(), spaceCamelCase, TX("$& ")).c_str()};
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

	bool CString::IsNumeric(const TChar* Str)
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

		static StringView sizes[]{"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

		const double scaleD    = Math::Log(double(size), 1024.l);
		const u32 scale        = u32(Math::FloorToI64(scaleD));
		const double finalSize = double(size) / Math::Pow(1024, scale);

		String sizeStr       = CString::Format("{:.1f}", finalSize);
		u32 numTrailingZeros = 0;
		for (u32 i = sizeStr.size() - 1; i >= 0; --i)
		{
			if (sizeStr[i] == '0')
			{
				++numTrailingZeros;
			}
			break;
		}
		CString::RemoveFromEnd(sizeStr, numTrailingZeros);

		return CString::Format("{}{}", sizeStr, sizes[scale]);
	}
}    // namespace Rift
