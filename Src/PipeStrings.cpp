// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "PipeStrings.h"

#include "Pipe/Core/Char.h"
#include "PipeMath.h"


namespace p::Strings
{

	String ToSentenceCase(StringView value)
	{
		String result;
		if (value.empty())
		{
			return result;
		}

		result.reserve(value.size());

		const char* p    = value.data();
		const char* end  = value.data() + value.size();
		const char* last = end - 1;
		for (; p < last; ++p)
		{
			const char* next = p + 1;
			if (FChar::IsAlpha(*p) && (FChar::IsUpper(*next) || FChar::IsDigit(*next)))
			{
				result.push_back(*p);
				result.push_back(' ');
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
		// Clamp to the string length to prevent underflowing the size
		str.resize(str.size() - Min(size, str.size()));
	}
	void RemoveFromEnd(String& str, StringView subStr)
	{
		if (EndsWith(str, subStr))
		{
			RemoveFromEnd(str, subStr.size());
		}
	}

	bool RemoveCharFromEnd(String& str, char c)
	{
		if (EndsWith(str, c))
		{
			str.resize(str.size() - 1);
			return true;
		}
		return false;
	}

	i32 Split(const String& str, TArray<String>& tokens, const char delim)
	{
		sizet current, previous = 0;
		current = str.find(delim);
		while (current != String::npos)
		{
			tokens.Add(str.substr(previous, current - previous));
			previous = current + 1;
			current  = str.find(delim, previous);
		}
		tokens.Add(str.substr(previous, current - previous));
		return tokens.Size();
	}

	bool Split(const String& str, String& a, String& b, const char* delim)
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

	bool IsNumeric(const char* str)
	{
		if (*str == '-' || *str == '+')
		{
			str++;
		}

		bool bHasDot = false;
		while (*str != '\0')
		{
			if (*str == '.')
			{
				if (bHasDot)
				{
					return false;
				}
				bHasDot = true;
			}
			else if (!FChar::IsDigit(*str))
			{
				return false;
			}

			++str;
		}

		return true;
	}

	String ParseMemorySize(sizet size, bool asBits)
	{
		String result;
		ParseMemorySizeTo(result, size, asBits);
		return result;
	}

	void ParseMemorySizeTo(String& str, sizet size, bool asBits)
	{
		if (size == 0)
		{
			str.append(asBits ? "0b" : "0B");
			return;
		}

		constexpr StringView byteSizes[]{"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
		constexpr StringView bitSizes[]{"b", "kb", "Mb", "Gb", "Tb", "Pb", "Eb", "Zb", "Yb"};

		const StringView* sizes = byteSizes;
		double dSize(size);
		if (asBits)
		{
			sizes = bitSizes;
			dSize *= 0.125;    // /8
		}
		const double dScale    = Log(dSize, 1024.l);
		const u32 scale        = u32(FloorToI64(dScale));
		const double finalSize = double(size) / Pow(1024, scale);

		FormatTo(str, StringView{"{:.1f}"}, finalSize);
		// Remove trailing zeros
		RemoveFromEnd(str, str.size() - Find(str, '0', FindDir::Back, true) - 1);
		RemoveFromEnd(str, str.size() - Find(str, '.', FindDir::Back, true) - 1);

		str.append(sizes[scale]);    // Suffix
	}

}    // namespace p::Strings
