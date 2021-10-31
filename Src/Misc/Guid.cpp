// Copyright 2015-2021 Piperift - All rights reserved

#include "Misc/Guid.h"

#include "Misc/Char.h"
#include "Platform/PlatformMisc.h"
#include "Serialization/Contexts.h"

#include <cstdio>


namespace Rift
{
	/* Guid interface
	 *****************************************************************************/

	void Guid::Read(Serl::ReadContext& ct)
	{
		ct.BeginObject();
		ct.Next(TX("a"), a);
		ct.Next(TX("b"), b);
		ct.Next(TX("c"), c);
		ct.Next(TX("d"), d);
	}

	void Guid::Write(Serl::WriteContext& ct) const
	{
		ct.BeginObject();
		ct.Next(TX("a"), a);
		ct.Next(TX("b"), b);
		ct.Next(TX("c"), c);
		ct.Next(TX("d"), d);
	}

	String Guid::ToString(EGuidFormats Format) const
	{
		switch (Format)
		{
			case EGuidFormats::DigitsWithHyphens:
				return Strings::Format(TX("{:08X}-{:04X}-{:04X}-{:04X}-{:04X}{:08X}"), a, b >> 16,
				    b & 0xFFFF, c >> 16, c & 0xFFFF, d);

			case EGuidFormats::DigitsWithHyphensInBraces:
				return Strings::Format(TX("{{{:08X}-{:04X}-{:04X}-{:04X}-{:04X}{:08X}}}"), a,
				    b >> 16, b & 0xFFFF, c >> 16, c & 0xFFFF, d);

			case EGuidFormats::DigitsWithHyphensInParentheses:
				return Strings::Format(TX("({:08X}-{:04X}-{:04X}-{:04X}-{:04X}{:08X})"), a, b >> 16,
				    b & 0xFFFF, c >> 16, c & 0xFFFF, d);

			case EGuidFormats::HexValuesInBraces:
				return Strings::Format(
				    TX("{{0x{:08X},0x{:04X},0x{:04X},{{0x{:02X},0x{:02X},0x{:02X},"
				       "0x{:02X},0x{:02X},0x{:02X},0x{:02X},0x{:02X}}}}}"),
				    a, b >> 16, b & 0xFFFF, c >> 24, (c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0XFF,
				    d >> 24, (d >> 16) & 0XFF, (d >> 8) & 0XFF, d & 0XFF);

			case EGuidFormats::UniqueObjectGuid:
				return Strings::Format(TX("{:08X}-{:08X}-{:08X}-{:08X}"), a, b, c, d);
			default: return Strings::Format(TX("{:08X}{:08X}{:08X}{:08X}"), a, b, c, d);
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
			if (guidString[0] == TX('{'))
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
			if ((GuidString[8] != TX('-')) || (GuidString[13] != TX('-'))
			    || (GuidString[18] != TX('-')) || (GuidString[23] != TX('-')))
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
			if ((GuidString[0] != TX('{')) || (GuidString[9] != TX('-'))
			    || (GuidString[14] != TX('-')) || (GuidString[19] != TX('-'))
			    || (GuidString[24] != TX('-')) || (GuidString[37] != TX('}')))
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
			if ((GuidString[0] != TX('(')) || (GuidString[9] != TX('-'))
			    || (GuidString[14] != TX('-')) || (GuidString[19] != TX('-'))
			    || (GuidString[24] != TX('-')) || (GuidString[37] != TX(')')))
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
			if ((GuidString[0] != TX('{')) || (GuidString[1] != TX('0'))
			    || (GuidString[2] != TX('x')) || (GuidString[11] != TX(','))
			    || (GuidString[12] != TX('0')) || (GuidString[13] != TX('x'))
			    || (GuidString[18] != TX(',')) || (GuidString[19] != TX('0'))
			    || (GuidString[20] != TX('x')) || (GuidString[25] != TX(','))
			    || (GuidString[26] != TX('{')) || (GuidString[27] != TX('0'))
			    || (GuidString[28] != TX('x')) || (GuidString[31] != TX(','))
			    || (GuidString[32] != TX('0')) || (GuidString[33] != TX('x'))
			    || (GuidString[36] != TX(',')) || (GuidString[37] != TX('0'))
			    || (GuidString[38] != TX('x')) || (GuidString[41] != TX(','))
			    || (GuidString[42] != TX('0')) || (GuidString[43] != TX('x'))
			    || (GuidString[46] != TX(',')) || (GuidString[47] != TX('0'))
			    || (GuidString[48] != TX('x')) || (GuidString[51] != TX(','))
			    || (GuidString[52] != TX('0')) || (GuidString[53] != TX('x'))
			    || (GuidString[56] != TX(',')) || (GuidString[57] != TX('0'))
			    || (GuidString[58] != TX('x')) || (GuidString[61] != TX(','))
			    || (GuidString[62] != TX('0')) || (GuidString[63] != TX('x'))
			    || (GuidString[66] != TX('}')) || (GuidString[67] != TX('}')))
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
			if ((GuidString[8] != TX('-')) || (GuidString[17] != TX('-'))
			    || (GuidString[26] != TX('-')))
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
}    // namespace Rift
