// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Core/String.h"

#include "Pipe/Core/Char.h"
#include "Pipe/Serialize/Serialization.h"
#include "PipeMath.h"


namespace p
{
	void Read(p::Reader& ct, p::String& val)
	{
		p::StringView view;
		ct.Serialize(view);
		val = view;
	}
	void Write(p::Writer& ct, const p::String& val)
	{
		ct.Serialize(p::StringView{val});
	}
}    // namespace p


namespace p::Strings
{

	String ToSentenceCase(StringView value)
	{
		if (value.empty())
		{
			return {};
		}

		String result;
		result.reserve(value.size());

		const TChar* p    = value.data();
		const TChar* end  = value.data() + value.size();
		const TChar* last = end - 1;
		for (; p < last; ++p)
		{
			const TChar* next = p + 1;
			if (FChar::IsAlpha(*p) && (FChar::IsUpper(*next) || FChar::IsDigit(*next)))
			{
				result.push_back(*p);
				result.push_back(TX(' '));
			}
			else
			{
				result.push_back(*p);
			}
		}
		result.push_back(*p);    // Last letter
		result[0] = FChar::ToUpper(result[0]);
		return result;
	}

	void RemoveFromStart(String& str, sizet size)
	{
		str.erase(0, size);
	}

	void RemoveFromEnd(String& str, sizet size)
	{
		str.resize(str.size() - size);
	}

	bool RemoveCharFromEnd(String& str, TChar c)
	{
		if (EndsWith(str, c))
		{
			str.resize(str.size() - 1);
			return true;
		}
		return false;
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

		const double scaleD    = Log(double(size), 1024.l);
		const u32 scale        = u32(FloorToI64(scaleD));
		const double finalSize = double(size) / Pow(1024, scale);

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
}    // namespace p::Strings
