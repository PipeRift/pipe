// Copyright 2015-2021 Piperift - All rights reserved

#include "Strings/String.h"

#include "Math/Math.h"
#include "Misc/Char.h"
#include "Serialization/Contexts.h"
#include "Strings/Regex.h"


namespace Rift
{
	void Read(Serl::ReadContext& ct, String& val)
	{
		StringView view;
		ct.Serialize(view);
		val = view;
	}
	void Write(Serl::WriteContext& ct, const String& val)
	{
		ct.Serialize(StringView{val});
	}
};    // namespace Rift

namespace Rift::Strings
{
	void ToSentenceCase(const String& str, String& result)
	{
		if (!str.empty())
		{
			// static const std::regex wordToCapital("\b[a-z]");
			static const Regex spaceCamelCase(TX("([a-zA-Z])(?=[A-Z0-9])"));

			result    = String{std::regex_replace(str.c_str(), spaceCamelCase, TX("$& ")).c_str()};
			result[0] = (char)::toupper(result[0]);
		}
		else
		{
			result = TX("");
		}
	}

	void RemoveFromStart(String& str, sizet size)
	{
		str.erase(0, size);
	}

	void RemoveFromEnd(String& str, sizet size)
	{
		str.erase(str.size() - 1 - size, str.size() - 1);
	}

	i32 Split(const String& str, TArray<String>& tokens, const TChar delim)
	{
		sizet current, previous = 0;
		current = str.find(delim);
		while (current != std::string::npos)
		{
			tokens.Add(str.substr(previous, current - previous));
			previous = current + 1;
			current  = str.find(delim, previous);
		}
		tokens.Add(str.substr(previous, current - previous));
		return tokens.Size();
	}

	bool Split(const String& str, String& a, String& b, const TChar* delim)
	{
		const sizet pos = str.find(delim);
		if (pos != String::npos)
		{
			a = str.substr(0, pos);
			b = str.substr(pos, str.size() - pos);
			return true;
		}
		return false;
	}

	bool IsNumeric(const String& str)
	{
		return IsNumeric(str.data());
	}

	bool IsNumeric(const TChar* Str)
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

	String ParseMemorySize(sizet size)
	{
		if (size <= 0)
		{
			return "0B";
		}

		static StringView sizes[]{"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

		const double scaleD    = Math::Log(double(size), 1024.l);
		const u32 scale        = u32(Math::FloorToI64(scaleD));
		const double finalSize = double(size) / Math::Pow(1024, scale);

		String sizeStr       = Format("{:.1f}", finalSize);
		u32 numTrailingZeros = 0;
		for (u32 i = u32(sizeStr.size()) - 1; i >= 0; --i)
		{
			if (sizeStr[i] == '0')
			{
				++numTrailingZeros;
			}
			break;
		}
		RemoveFromEnd(sizeStr, numTrailingZeros);

		return Format("{}{}", sizeStr, sizes[scale]);
	}
}    // namespace Rift::Strings
