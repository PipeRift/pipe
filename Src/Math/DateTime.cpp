// Copyright 2015-2022 Piperift - All rights reserved

#include "Math/DateTime.h"

#include "Core/Char.h"
#include "Log.h"


namespace p
{
	/* DateTime constants
	 *****************************************************************************/

	const i32 DateTime::DaysPerMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	const i32 DateTime::DaysToMonth[]  = {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};


	/* DateTime Constructors
	 *****************************************************************************/

	DateTime::DateTime(
	    i32 year, i32 month, i32 day, i32 hour, i32 minute, i32 second, i32 millisecond)
	{
		if (!Validate(year, month, day, hour, minute, second, millisecond))
		{
			// LOG Invalid date-time
			Log::Warning("Created invalid date format.");
		}

		value = SysTime{SysDays(Year{year} / month / day) + Hours{hour} + Minutes{minute}
		                + Seconds{second} + Milliseconds{millisecond}};
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
		auto timeDays = Chrono::floor<Days>(value);
		const YearMonthDay ymd{timeDays};

		return (timeDays.time_since_epoch() - LocalDays{ymd.year() / Jan / 1}.time_since_epoch())
		    .count();
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
		return (u32)YearMonthDay{Chrono::floor<Days>(value)}.month();
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
			case DayOfWeek::Monday: DayStr = TX("Mon"); break;
			case DayOfWeek::Tuesday: DayStr = TX("Tue"); break;
			case DayOfWeek::Wednesday: DayStr = TX("Wed"); break;
			case DayOfWeek::Thursday: DayStr = TX("Thu"); break;
			case DayOfWeek::Friday: DayStr = TX("Fri"); break;
			case DayOfWeek::Saturday: DayStr = TX("Sat"); break;
			case DayOfWeek::Sunday: DayStr = TX("Sun"); break;
		}

		switch (GetMonthOfYear())
		{
			case MonthOfYear::January: MonthStr = TX("Jan"); break;
			case MonthOfYear::February: MonthStr = TX("Feb"); break;
			case MonthOfYear::March: MonthStr = TX("Mar"); break;
			case MonthOfYear::April: MonthStr = TX("Apr"); break;
			case MonthOfYear::May: MonthStr = TX("May"); break;
			case MonthOfYear::June: MonthStr = TX("Jun"); break;
			case MonthOfYear::July: MonthStr = TX("Jul"); break;
			case MonthOfYear::August: MonthStr = TX("Aug"); break;
			case MonthOfYear::September: MonthStr = TX("Sep"); break;
			case MonthOfYear::October: MonthStr = TX("Oct"); break;
			case MonthOfYear::November: MonthStr = TX("Nov"); break;
			case MonthOfYear::December: MonthStr = TX("Dec"); break;
		}

		return Strings::Format(TX("{}, {:02d} {} {} {:02i}:{:02i}:{:02i} GMT"), DayStr.c_str(),
		    GetDay(), MonthStr.c_str(), GetYear(), GetHour(), GetMinute(), GetSecond());
	}


	String DateTime::ToIso8601() const
	{
		return ToString(TX("%Y-%m-%dT%H:%M:%S.%sZ"));
	}


	String DateTime::ToString() const
	{
		return ToString(TX("%Y.%m.%d-%H.%M.%S"));
	}


	String DateTime::ToString(const TChar* format) const
	{
		// return Strings::Format(format, *value);
		String result;

		if (format != nullptr)
		{
			while (*format != TX('\0'))
			{
				if ((*format == TX('%')) && (*(++format) != TX('\0')))
				{
					switch (*format)
					{
						case TX('a'): result += IsMorning() ? TX("am") : TX("pm"); break;
						case TX('A'): result += IsMorning() ? TX("AM") : TX("PM"); break;
						case TX('d'): Strings::FormatTo(result, TX("{:02i}"), GetDay()); break;
						case TX('D'):
							Strings::FormatTo(result, TX("{:03i}"), GetDayOfYear());
							break;
						case TX('m'): Strings::FormatTo(result, TX("{:02i}"), GetMonth()); break;
						case TX('y'):
							Strings::FormatTo(result, TX("{:02i}"), GetYear() % 100);
							break;
						case TX('Y'): Strings::FormatTo(result, TX("{:04i}"), GetYear()); break;
						case TX('h'): Strings::FormatTo(result, TX("{:02i}"), GetHour12()); break;
						case TX('H'): Strings::FormatTo(result, TX("{:02i}"), GetHour()); break;
						case TX('M'): Strings::FormatTo(result, TX("{:02i}"), GetMinute()); break;
						case TX('S'): Strings::FormatTo(result, TX("{:02i}"), GetSecond()); break;
						case TX('s'):
							Strings::FormatTo(result, TX("{:03i}"), GetMillisecond());
							break;
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
		assert((Month >= 1) && (Month <= 12));

		if ((Month == 2) && IsLeapYear(Year))
		{
			return 29;
		}

		return DaysPerMonth[Month];
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
		Strings::Split(fixedString, Tokens, TX(' '));

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

		if ((Next <= ptr) || (*Next == TX('\0')))
		{
			return false;
		}

		ptr   = Next + 1;    // skip separator
		Month = FChar::StrtoI32(ptr, &Next, 10);

		if ((Next <= ptr) || (*Next == TX('\0')))
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
		if (*Next == TX('T'))
		{
			ptr = Next + 1;

			// parse time
			Hour = FChar::StrtoI32(ptr, &Next, 10);

			if ((Next <= ptr) || (*Next == TX('\0')))
			{
				return false;
			}

			ptr    = Next + 1;    // skip separator
			Minute = FChar::StrtoI32(ptr, &Next, 10);

			if ((Next <= ptr) || (*Next == TX('\0')))
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
			if (*Next == TX('.'))
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
			if (*Next == TX('+') || *Next == TX('-'))
			{
				// include the separator since it's + or -
				ptr = Next;

				// parse the timezone offset
				TzHour = FChar::StrtoI32(ptr, &Next, 10);

				if ((Next <= ptr) || (*Next == TX('\0')))
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
			else if ((*Next != TX('\0')) && (*Next != TX('Z')))
			{
				return false;
			}
		}
		else if (*Next != TX('\0'))
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
}    // namespace p
