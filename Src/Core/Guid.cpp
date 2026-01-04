// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Pipe/Core/Guid.h"

#include "Pipe/Core/Char.h"
#include "PipePlatform.h"

#include <cstdio>


namespace p
{
	/* Guid interface
	 *****************************************************************************/

	String Guid::ToString(EGuidFormats Format) const
	{
		switch (Format)
		{
			case EGuidFormats::DigitsWithHyphens:
				return Strings::Format("{:08X}-{:04X}-{:04X}-{:04X}-{:04X}{:08X}", a, b >> 16,
				    b & 0xFFFF, c >> 16, c & 0xFFFF, d);

			case EGuidFormats::DigitsWithHyphensInBraces:
				return Strings::Format("{{{:08X}-{:04X}-{:04X}-{:04X}-{:04X}{:08X}}}", a, b >> 16,
				    b & 0xFFFF, c >> 16, c & 0xFFFF, d);

			case EGuidFormats::DigitsWithHyphensInParentheses:
				return Strings::Format("({:08X}-{:04X}-{:04X}-{:04X}-{:04X}{:08X})", a, b >> 16,
				    b & 0xFFFF, c >> 16, c & 0xFFFF, d);

			case EGuidFormats::HexValuesInBraces:
				return Strings::Format(
				    "{{0x{:08X},0x{:04X},0x{:04X},{{0x{:02X},0x{:02X},0x{:02X},"
				    "0x{:02X},0x{:02X},0x{:02X},0x{:02X},0x{:02X}}}}}",
				    a, b >> 16, b & 0xFFFF, c >> 24, (c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0XFF,
				    d >> 24, (d >> 16) & 0XFF, (d >> 8) & 0XFF, d & 0XFF);

			case EGuidFormats::UniqueObjectGuid:
				return Strings::Format("{:08X}-{:08X}-{:08X}-{:08X}", a, b, c, d);
			default: return Strings::Format("{:08X}{:08X}{:08X}{:08X}", a, b, c, d);
		}
	}


	/* Guid static interface
	 *****************************************************************************/

	Guid Guid::New()
	{
		Guid result(0, 0, 0, 0);
		PlatformMisc::CreateGuid(result);
		return result;
	}


	bool Guid::Parse(const String& guidString, Guid& OutGuid)
	{
		const sizet stringSize = guidString.size();
		if (stringSize == 32)
		{
			return ParseExact(guidString, EGuidFormats::Digits, OutGuid);
		}

		if (stringSize == 36)
		{
			return ParseExact(guidString, EGuidFormats::DigitsWithHyphens, OutGuid);
		}

		if (stringSize == 38)
		{
			if (guidString[0] == '{')
			{
				return ParseExact(guidString, EGuidFormats::DigitsWithHyphensInBraces, OutGuid);
			}

			return ParseExact(guidString, EGuidFormats::DigitsWithHyphensInParentheses, OutGuid);
		}

		if (stringSize == 68)
		{
			return ParseExact(guidString, EGuidFormats::HexValuesInBraces, OutGuid);
		}

		if (stringSize == 35)
		{
			return ParseExact(guidString, EGuidFormats::UniqueObjectGuid, OutGuid);
		}

		return false;
	}


	bool Guid::ParseExact(const String& GuidString, EGuidFormats Format, Guid& /*OutGuid*/)
	{
		String NormalizedGuidString;
		NormalizedGuidString.reserve(32);

		if (Format == EGuidFormats::Digits)
		{
			NormalizedGuidString = GuidString;
		}
		else if (Format == EGuidFormats::DigitsWithHyphens)
		{
			if ((GuidString[8] != '-') || (GuidString[13] != '-') || (GuidString[18] != '-')
			    || (GuidString[23] != '-'))
			{
				return false;
			}

			NormalizedGuidString += GuidString.substr(0, 8);
			NormalizedGuidString += GuidString.substr(9, 4);
			NormalizedGuidString += GuidString.substr(14, 4);
			NormalizedGuidString += GuidString.substr(19, 4);
			NormalizedGuidString += GuidString.substr(24, 12);
		}
		else if (Format == EGuidFormats::DigitsWithHyphensInBraces)
		{
			if ((GuidString[0] != '{') || (GuidString[9] != '-') || (GuidString[14] != '-')
			    || (GuidString[19] != '-') || (GuidString[24] != '-') || (GuidString[37] != '}'))
			{
				return false;
			}

			NormalizedGuidString += GuidString.substr(1, 8);
			NormalizedGuidString += GuidString.substr(10, 4);
			NormalizedGuidString += GuidString.substr(15, 4);
			NormalizedGuidString += GuidString.substr(20, 4);
			NormalizedGuidString += GuidString.substr(25, 12);
		}
		else if (Format == EGuidFormats::DigitsWithHyphensInParentheses)
		{
			if ((GuidString[0] != '(') || (GuidString[9] != '-') || (GuidString[14] != '-')
			    || (GuidString[19] != '-') || (GuidString[24] != '-') || (GuidString[37] != ')'))
			{
				return false;
			}

			NormalizedGuidString += GuidString.substr(1, 8);
			NormalizedGuidString += GuidString.substr(10, 4);
			NormalizedGuidString += GuidString.substr(15, 4);
			NormalizedGuidString += GuidString.substr(20, 4);
			NormalizedGuidString += GuidString.substr(25, 12);
		}
		else if (Format == EGuidFormats::HexValuesInBraces)
		{
			if ((GuidString[0] != '{') || (GuidString[1] != '0') || (GuidString[2] != 'x')
			    || (GuidString[11] != ',') || (GuidString[12] != '0') || (GuidString[13] != 'x')
			    || (GuidString[18] != ',') || (GuidString[19] != '0') || (GuidString[20] != 'x')
			    || (GuidString[25] != ',') || (GuidString[26] != '{') || (GuidString[27] != '0')
			    || (GuidString[28] != 'x') || (GuidString[31] != ',') || (GuidString[32] != '0')
			    || (GuidString[33] != 'x') || (GuidString[36] != ',') || (GuidString[37] != '0')
			    || (GuidString[38] != 'x') || (GuidString[41] != ',') || (GuidString[42] != '0')
			    || (GuidString[43] != 'x') || (GuidString[46] != ',') || (GuidString[47] != '0')
			    || (GuidString[48] != 'x') || (GuidString[51] != ',') || (GuidString[52] != '0')
			    || (GuidString[53] != 'x') || (GuidString[56] != ',') || (GuidString[57] != '0')
			    || (GuidString[58] != 'x') || (GuidString[61] != ',') || (GuidString[62] != '0')
			    || (GuidString[63] != 'x') || (GuidString[66] != '}') || (GuidString[67] != '}'))
			{
				return false;
			}

			NormalizedGuidString += GuidString.substr(3, 8);
			NormalizedGuidString += GuidString.substr(14, 4);
			NormalizedGuidString += GuidString.substr(21, 4);
			NormalizedGuidString += GuidString.substr(29, 2);
			NormalizedGuidString += GuidString.substr(34, 2);
			NormalizedGuidString += GuidString.substr(39, 2);
			NormalizedGuidString += GuidString.substr(44, 2);
			NormalizedGuidString += GuidString.substr(49, 2);
			NormalizedGuidString += GuidString.substr(54, 2);
			NormalizedGuidString += GuidString.substr(59, 2);
			NormalizedGuidString += GuidString.substr(64, 2);
		}
		else if (Format == EGuidFormats::UniqueObjectGuid)
		{
			if ((GuidString[8] != '-') || (GuidString[17] != '-') || (GuidString[26] != '-'))
			{
				return false;
			}

			NormalizedGuidString += GuidString.substr(0, 8);
			NormalizedGuidString += GuidString.substr(9, 8);
			NormalizedGuidString += GuidString.substr(18, 8);
			NormalizedGuidString += GuidString.substr(27, 8);
		}

		for (char Index : NormalizedGuidString)
		{
			if (!FChar::IsHexDigit(Index))
			{
				return false;
			}
		}

		// #TODO: Implement HexNumber parse
		/*OutGuid = Guid(
		    FParse::HexNumber(NormalizedGuidString.substr(0, 8)),
		    FParse::HexNumber(NormalizedGuidString.substr(8, 8)),
		    FParse::HexNumber(NormalizedGuidString.substr(16, 8)),
		    FParse::HexNumber(NormalizedGuidString.substr(24, 8))
		);*/

		return true;
	}
}    // namespace p
