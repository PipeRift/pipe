// Copyright 2015-2022 Piperift - All rights reserved

#include "Misc/Timespan.h"


namespace Rift
{
	/* FTimespan interface
	 *****************************************************************************/

	String Timespan::ToString() const
	{
		if (GetDays() == 0)
		{
			return ToString(TX("%h:%m:%s.%f"));
		}

		return ToString(TX("%d.%h:%m:%s.%f"));
	}


	String Timespan::ToString(const TChar* format) const
	{
		String result;

		result += (*this < Timespan::Zero()) ? TX('-') : TX('+');

		while (*format != TX('\0'))
		{
			if ((*format == TX('%')) && (*++format != TX('\0')))
			{
				switch (*format)
				{
					case TX('d'): Strings::FormatTo(result, TX("{}"), Math::Abs(GetDays())); break;
					case TX('D'):
						Strings::FormatTo(result, TX("{:08i}"), Math::Abs(GetDays()));
						break;
					case TX('h'):
						Strings::FormatTo(result, TX("{:02i}"), Math::Abs(GetHours()));
						break;
					case TX('m'):
						Strings::FormatTo(result, TX("{:02i}"), Math::Abs(GetMinutes()));
						break;
					case TX('s'):
						Strings::FormatTo(result, TX("{:02i}"), Math::Abs(GetSeconds()));
						break;
					case TX('f'):
						Strings::FormatTo(result, TX("{:03i}"), Math::Abs(GetFractionMilli()));
						break;
					case TX('u'):
						Strings::FormatTo(result, TX("{:06i}"), Math::Abs(GetFractionMicro()));
						break;
					case TX('t'):
						Strings::FormatTo(result, TX("{:07i}"), Math::Abs(GetFractionTicks()));
						break;
					case TX('n'):
						Strings::FormatTo(result, TX("{:09i}"), Math::Abs(GetFractionNano()));
						break;
					default: result += *format;
				}
			}
			else
			{
				result += *format;
			}

			++format;
		}

		return result;
	}


	/* FTimespan static interface
	 *****************************************************************************/

	bool Timespan::Parse(const String& TimespanString, Timespan& OutTimespan)
	{
		// @todo gmp: implement stricter FTimespan parsing; this implementation is too forgiving

		// get string tokens
		const bool HasFractional = Strings::Contains(TimespanString, TX('.'))
		                        || Strings::Contains(TimespanString, TX(','));
		String TokenString = TimespanString;
		Strings::Replace(TokenString, TX('.'), TX(':'));
		Strings::Replace(TokenString, TX(','), TX(':'));

		const bool Negative = TokenString[0] == TX('-');
		Strings::Replace(TokenString, TX('-'), TX(':'));
		Strings::Replace(TokenString, TX('+'), TX(':'));

		TArray<String> Tokens;
		Strings::Split(TokenString, Tokens, TX(':'));

		if (!HasFractional)
		{
			Tokens.AddDefaulted();
		}

		// poor man's token verification
		for (const String& token : Tokens)
		{
			if (!token.empty() && !Strings::IsNumeric(token))
			{
				return false;
			}
		}

		// add missing tokens
		if (Tokens.Size() < 5)
		{
			Tokens.InsertDefaulted(0, 5 - Tokens.Size());
		}
		else if (Tokens.Size() > 5)
		{
			return false;
		}

		// pad fractional token with zeros
		if (HasFractional)
		{
			const i32 FractionalLen = (i32)(Tokens[4].size());

			if (FractionalLen > 9)
			{
				Tokens[4] = Strings::FrontSubstr(Tokens[4], 9);
			}
			else if (FractionalLen < 9)
			{
				Tokens[4] += Strings::FrontSubstr({TX("000000000")}, 9 - FractionalLen);
			}
		}

		const i32 days         = *Strings::ToI32(Tokens[0]);
		const i32 hours        = *Strings::ToI32(Tokens[1]);
		const i32 minutes      = *Strings::ToI32(Tokens[2]);
		const i32 seconds      = *Strings::ToI32(Tokens[3]);
		const i32 fractionNano = *Strings::ToI32(Tokens[4]);

		// Max days
		if ((days > Chrono::floor<Days>(DecMicroseconds::max()).count() - 1))
		{
			return false;
		}

		if ((hours > 23) || (minutes > 59) || (seconds > 59) || (fractionNano > 999999999))
		{
			return false;
		}

		OutTimespan.Assign(days, hours, minutes, seconds, fractionNano);

		if (Negative)
		{
			OutTimespan.duration *= -1;
		}

		return true;
	}


	/* Timespan implementation
	 *****************************************************************************/

	void Timespan::Assign(i32 days, i32 hours, i32 minutes, i32 seconds, i32 fractionNano)
	{
		duration = Chrono::floor<DecMicroseconds>(
		    Days{days} + Chrono::hours{hours} + Chrono::minutes{minutes} + Chrono::seconds{seconds}
		    + Chrono::nanoseconds{fractionNano});
	}

	Timespan Timespan::FromHours(i32 hours)
	{
		return Timespan{hours, 0, 0};
	}

	Timespan Timespan::FromMinutes(i32 minutes)
	{
		return Timespan{0, minutes, 0};
	}

	Timespan Timespan::FromSeconds(i32 seconds)
	{
		return Timespan{0, 0, seconds};
	}
}    // namespace Rift
