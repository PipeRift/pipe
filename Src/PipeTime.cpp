// Copyright 2015-2024 Piperift - All rights reserved

#include "PipeTime.h"

#include "Pipe/Core/Char.h"
#include "Pipe/Core/Log.h"

#include <thread>


namespace p
{
	String Timespan::ToString() const
	{
		if (GetDays() == 0)
		{
			return ToString("%h:%m:%s.%f");
		}

		return ToString("%d.%h:%m:%s.%f");
	}


	String Timespan::ToString(const TChar* format) const
	{
		String result;

		result += (*this < Timespan::Zero()) ? '-' : '+';

		while (*format != '\0')
		{
			if ((*format == '%') && (*++format != '\0'))
			{
				switch (*format)
				{
					case 'd': Strings::FormatTo(result, "{}", Abs(GetDays())); break;
					case 'D': Strings::FormatTo(result, "{:08i}", Abs(GetDays())); break;
					case 'h': Strings::FormatTo(result, "{:02i}", Abs(GetHours())); break;
					case 'm': Strings::FormatTo(result, "{:02i}", Abs(GetMinutes())); break;
					case 's': Strings::FormatTo(result, "{:02i}", Abs(GetSeconds())); break;
					case 'f': Strings::FormatTo(result, "{:03i}", Abs(GetFractionMilli())); break;
					case 'u': Strings::FormatTo(result, "{:06i}", Abs(GetFractionMicro())); break;
					case 't': Strings::FormatTo(result, "{:07i}", Abs(GetFractionTicks())); break;
					case 'n': Strings::FormatTo(result, "{:09i}", Abs(GetFractionNano())); break;
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
		const bool HasFractional =
		    Strings::Contains(TimespanString, '.') || Strings::Contains(TimespanString, ',');
		String TokenString = TimespanString;
		Strings::Replace(TokenString, '.', ':');
		Strings::Replace(TokenString, ',', ':');

		const bool Negative = TokenString[0] == '-';
		Strings::Replace(TokenString, '-', ':');
		Strings::Replace(TokenString, '+', ':');

		TArray<String> Tokens;
		Strings::Split(TokenString, Tokens, ':');

		if (!HasFractional)
		{
			Tokens.Add();
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
			Tokens.Insert(0, 5 - Tokens.Size(), {});
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
				Tokens[4] += Strings::FrontSubstr({"000000000"}, 9 - FractionalLen);
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
		duration = Chrono::floor<DecMicroseconds>(Days{days} + Hours{hours} + Minutes{minutes}
		                                          + Seconds{seconds} + Nanoseconds{fractionNano});
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


	/* DateTime constants
	 *****************************************************************************/

	const u32 DateTime::daysPerMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	const u32 DateTime::daysToMonth[]  = {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};


	/* DateTime Constructors
	 *****************************************************************************/

	DateTime::DateTime(
	    i32 year, i32 month, i32 day, i32 hour, i32 minute, i32 second, i32 millisecond)
	{
		if (!Validate(year, month, day, hour, minute, second, millisecond))
		{
			// LOG Invalid date-time
			Warning("Created invalid date format.");
		}

		year -= month <= 2;
		const int century = (year >= 0 ? year : year - 99) / 100;
		month += (month > 2 ? static_cast<unsigned int>(-3) : 9);    // [0, 11]
		const int dayOfYear = static_cast<int>(((979 * month + 19) >> 5) + day) - 1;
		auto daysSinceCivil =
		    Days{((1461 * year) >> 2) - century + (century >> 2) + dayOfYear - 719468};

		value = SysTime{daysSinceCivil + Hours{hour} + Minutes{minute} + Seconds{second}
		                + Milliseconds{millisecond}};
	}


	/* DateTime interface
	 *****************************************************************************/


	YearMonthDay DateTime::GetDateComponents() const
	{
		return {Chrono::floor<Days>(value)};
	}

	u32 DateTime::GetDay() const
	{
		return (u32)GetDateComponents().day();
	}

	DayOfWeek DateTime::GetDayOfWeek() const
	{
		const WeekDay wd{Chrono::floor<Days>(value)};

		// January 1, 0001 was a Monday
		return static_cast<DayOfWeek>((wd - Sunday).count());
	}

	u32 DateTime::GetDayOfYear() const
	{
		const YearMonthDay ymd = GetDateComponents();

		return daysToMonth[u32(ymd.month()) - 1] + u32(ymd.day());
	}


	u32 DateTime::GetHour12() const
	{
		u32 Hour = GetHour();

		if (Hour < 1)
			return 12;

		if (Hour > 12)
			return (Hour - 12);

		return Hour;
	}


	u32 DateTime::GetMonth() const
	{
		return (u32)GetDateComponents().month();
	}

	i32 DateTime::GetYear() const
	{
		return (i32)GetDateComponents().year();
	}

	String DateTime::ToHttpDate() const
	{
		String DayStr;
		String MonthStr;

		switch (GetDayOfWeek())
		{
			case DayOfWeek::Monday: DayStr = "Mon"; break;
			case DayOfWeek::Tuesday: DayStr = "Tue"; break;
			case DayOfWeek::Wednesday: DayStr = "Wed"; break;
			case DayOfWeek::Thursday: DayStr = "Thu"; break;
			case DayOfWeek::Friday: DayStr = "Fri"; break;
			case DayOfWeek::Saturday: DayStr = "Sat"; break;
			case DayOfWeek::Sunday: DayStr = "Sun"; break;
		}

		switch (GetMonthOfYear())
		{
			case MonthOfYear::January: MonthStr = "Jan"; break;
			case MonthOfYear::February: MonthStr = "Feb"; break;
			case MonthOfYear::March: MonthStr = "Mar"; break;
			case MonthOfYear::April: MonthStr = "Apr"; break;
			case MonthOfYear::May: MonthStr = "May"; break;
			case MonthOfYear::June: MonthStr = "Jun"; break;
			case MonthOfYear::July: MonthStr = "Jul"; break;
			case MonthOfYear::August: MonthStr = "Aug"; break;
			case MonthOfYear::September: MonthStr = "Sep"; break;
			case MonthOfYear::October: MonthStr = "Oct"; break;
			case MonthOfYear::November: MonthStr = "Nov"; break;
			case MonthOfYear::December: MonthStr = "Dec"; break;
		}

		return Strings::Format("{}, {:02d} {} {} {:02i}:{:02i}:{:02i} GMT", DayStr, GetDay(),
		    MonthStr, GetYear(), GetHour(), GetMinute(), GetSecond());
	}


	String DateTime::ToIso8601() const
	{
		return ToString("%Y-%m-%dT%H:%M:%S.%sZ");
	}


	String DateTime::ToString() const
	{
		return ToString("%Y.%m.%d-%H.%M.%S");
	}


	String DateTime::ToString(const TChar* format) const
	{
		// return Strings::Format(format, *value);
		String result;

		if (format != nullptr)
		{
			while (*format != '\0')
			{
				if ((*format == '%') && (*(++format) != '\0'))
				{
					switch (*format)
					{
						case 'a': result += IsMorning() ? "am" : "pm"; break;
						case 'A': result += IsMorning() ? "AM" : "PM"; break;
						case 'd': Strings::FormatTo(result, "{:02i}", GetDay()); break;
						case 'D': Strings::FormatTo(result, "{:03i}", GetDayOfYear()); break;
						case 'm': Strings::FormatTo(result, "{:02i}", GetMonth()); break;
						case 'y': Strings::FormatTo(result, "{:02i}", GetYear() % 100); break;
						case 'Y': Strings::FormatTo(result, "{:04i}", GetYear()); break;
						case 'h': Strings::FormatTo(result, "{:02i}", GetHour12()); break;
						case 'H': Strings::FormatTo(result, "{:02i}", GetHour()); break;
						case 'M': Strings::FormatTo(result, "{:02i}", GetMinute()); break;
						case 'S': Strings::FormatTo(result, "{:02i}", GetSecond()); break;
						case 's': Strings::FormatTo(result, "{:03i}", GetMillisecond()); break;
						default: result += *format;
					}
				}
				else
				{
					result += *format;
				}

				// move to the next one
				++format;
			}
		}
		return result;
	}


	/* DateTime static interface
	 *****************************************************************************/

	i32 DateTime::DaysInMonth(i32 Year, i32 Month)
	{
		Check((Month >= 1) && (Month <= 12));

		if ((Month == 2) && IsLeapYear(Year))
		{
			return 29;
		}

		return daysPerMonth[Month];
	}

	i32 DateTime::DaysInYear(i32 Year)
	{
		return IsLeapYear(Year) ? 366 : 365;
	}

	bool DateTime::IsLeapYear(i32 Year)
	{
		if ((Year % 4) == 0)
		{
			return (((Year % 100) != 0) || ((Year % 400) == 0));
		}

		return false;
	}

	DateTime DateTime::Now()
	{
		return {Chrono::floor<SysTime::duration>(SysClock::now())};
	}

	bool DateTime::Parse(const String& dateTimeString, DateTime& OutDateTime)
	{
		const String& fixedString = dateTimeString;
		// first replace -, : and . with space
		Strings::Replace(fixedString, '-', ' ');
		Strings::Replace(fixedString, ':', ' ');
		Strings::Replace(fixedString, '.', ' ');

		// split up on a single delimiter
		TArray<String> Tokens;
		Strings::Split(fixedString, Tokens, ' ');

		// make sure it parsed it properly (within reason)
		if ((Tokens.Size() < 6) || (Tokens.Size() > 7))
		{
			return false;
		}

		const i32 Year        = *Strings::ToI32(Tokens[0]);
		const i32 Month       = *Strings::ToI32(Tokens[1]);
		const i32 Day         = *Strings::ToI32(Tokens[2]);
		const i32 Hour        = *Strings::ToI32(Tokens[3]);
		const i32 Minute      = *Strings::ToI32(Tokens[4]);
		const i32 Second      = *Strings::ToI32(Tokens[5]);
		const i32 Millisecond = Tokens.Size() > 6 ? *Strings::ToI32(Tokens[6]) : 0;

		if (!Validate(Year, Month, Day, Hour, Minute, Second, Millisecond))
		{
			return false;
		}

		// convert the tokens to numbers
		OutDateTime = {Year, Month, Day, Hour, Minute, Second, Millisecond};

		return true;
	}

	bool DateTime::ParseIso8601(const TChar* DateTimeString, DateTime& OutDateTime)
	{
		// DateOnly: YYYY-MM-DD
		// DateTime: YYYY-mm-ddTHH:MM:SS(.ssss)(Z|+th:tm|-th:tm)

		const TChar* ptr = DateTimeString;
		TChar* Next      = nullptr;

		i32 Year = 0, Month = 0, Day = 0;
		i32 Hour = 0, Minute = 0, Second = 0, Millisecond = 0;
		i32 TzHour = 0, TzMinute = 0;

		// get date
		Year = FChar::StrtoI32(ptr, &Next, 10);

		if ((Next <= ptr) || (*Next == '\0'))
		{
			return false;
		}

		ptr   = Next + 1;    // skip separator
		Month = FChar::StrtoI32(ptr, &Next, 10);

		if ((Next <= ptr) || (*Next == '\0'))
		{
			return false;
		}

		ptr = Next + 1;    // skip separator
		Day = FChar::StrtoI32(ptr, &Next, 10);

		if (Next <= ptr)
		{
			return false;
		}

		// check whether this is date and time
		if (*Next == 'T')
		{
			ptr = Next + 1;

			// parse time
			Hour = FChar::StrtoI32(ptr, &Next, 10);

			if ((Next <= ptr) || (*Next == '\0'))
			{
				return false;
			}

			ptr    = Next + 1;    // skip separator
			Minute = FChar::StrtoI32(ptr, &Next, 10);

			if ((Next <= ptr) || (*Next == '\0'))
			{
				return false;
			}

			ptr    = Next + 1;    // skip separator
			Second = FChar::StrtoI32(ptr, &Next, 10);

			if (Next <= ptr)
			{
				return false;
			}

			// check for milliseconds
			if (*Next == '.')
			{
				ptr         = Next + 1;
				Millisecond = FChar::StrtoI32(ptr, &Next, 10);

				// should be no more than 3 digits
				if ((Next <= ptr) || (Next > ptr + 3))
				{
					return false;
				}

				for (sizet Digits = Next - ptr; Digits < 3; ++Digits)
				{
					Millisecond *= 10;
				}
			}

			// see if the timezone offset is included
			if (*Next == '+' || *Next == '-')
			{
				// include the separator since it's + or -
				ptr = Next;

				// parse the timezone offset
				TzHour = FChar::StrtoI32(ptr, &Next, 10);

				if ((Next <= ptr) || (*Next == '\0'))
				{
					return false;
				}

				ptr      = Next + 1;    // skip separator
				TzMinute = FChar::StrtoI32(ptr, &Next, 10);

				if (Next <= ptr)
				{
					return false;
				}
			}
			else if ((*Next != '\0') && (*Next != 'Z'))
			{
				return false;
			}
		}
		else if (*Next != '\0')
		{
			return false;
		}

		if (!Validate(Year, Month, Day, Hour, Minute, Second, Millisecond))
		{
			return false;
		}

		DateTime Final(Year, Month, Day, Hour, Minute, Second, Millisecond);

		// adjust for the timezone (bringing the DateTime into UTC)
		i32 TzOffsetMinutes = (TzHour < 0) ? TzHour * 60 - TzMinute : TzHour * 60 + TzMinute;
		Final -= Timespan::FromMinutes(TzOffsetMinutes);
		OutDateTime = Final;

		return true;
	}

	bool DateTime::Validate(
	    i32 Year, i32 Month, i32 Day, i32 Hour, i32 Minute, i32 Second, i32 Millisecond)
	{
		return (Year >= 1) && (Year <= 9999) && (Month >= 1) && (Month <= 12) && (Day >= 1)
		    && (Day <= DaysInMonth(Year, Month)) && (Hour >= 0) && (Hour <= 23) && (Minute >= 0)
		    && (Minute <= 59) && (Second >= 0) && (Second <= 59) && (Millisecond >= 0)
		    && (Millisecond <= 999);
	}


	void FrameTime::PreTick()
	{
		previousTime = currentTime;
		currentTime  = DateTime::Now();

		// Avoid too big delta times
		realDeltaTime = Min(0.15f, (currentTime - previousTime).GetTotalSeconds());

		// Apply time dilation
		deltaTime = realDeltaTime * timeDilation;
	}

	void FrameTime::PostTick()
	{
		const float secondsPassed      = (DateTime::Now() - currentTime).GetTotalSeconds();
		const float extraTimeForFPSCAP = minFrameTime - secondsPassed;
		if (extraTimeForFPSCAP > 0.0f)
		{
			Chrono::duration<float, Chrono::seconds::period> sleepPeriod{extraTimeForFPSCAP};
			std::this_thread::sleep_for(sleepPeriod);
		}
	}
}    // namespace p
