// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/String.h"

// TODO: Move all chrono & date uses to cpp
#include <date/date.h>

#include <chrono>


namespace p
{
	namespace Chrono = std::chrono;

	// decimals of microseconds
	using DecMicroseconds = Chrono::duration<i64, std::ratio<1, 10'000'000>>;

	using SysClock    = Chrono::system_clock;
	using SysTime     = Chrono::time_point<SysClock, DecMicroseconds>;
	using SysDuration = Chrono::duration<SysClock, DecMicroseconds>;


	using Nanoseconds  = Chrono::nanoseconds;
	using Microseconds = Chrono::microseconds;
	using Milliseconds = Chrono::milliseconds;
	using Seconds      = Chrono::seconds;
	using Minutes      = Chrono::minutes;
	using Hours        = Chrono::hours;

	using YearMonthDay = date::year_month_day;
	using Day          = date::day;
	using Days         = date::days;
	using Month        = date::month;
	using Months       = date::months;
	using Year         = date::year;
	using Years        = date::years;
	using WeekDay      = date::weekday;
	using LocalDays    = date::local_days;

	using SysDays = date::sys_days;

	constexpr const Month January   = date::January;
	constexpr const Month February  = date::February;
	constexpr const Month March     = date::March;
	constexpr const Month April     = date::April;
	constexpr const Month May       = date::May;
	constexpr const Month June      = date::June;
	constexpr const Month July      = date::July;
	constexpr const Month August    = date::August;
	constexpr const Month September = date::September;
	constexpr const Month October   = date::October;
	constexpr const Month November  = date::November;
	constexpr const Month December  = date::December;

	constexpr const WeekDay Monday    = date::Monday;
	constexpr const WeekDay Tuesday   = date::Tuesday;
	constexpr const WeekDay Wednesday = date::Wednesday;
	constexpr const WeekDay Thursday  = date::Thursday;
	constexpr const WeekDay Friday    = date::Friday;
	constexpr const WeekDay Saturday  = date::Saturday;
	constexpr const WeekDay Sunday    = date::Sunday;

	constexpr const Month Jan = date::jan;
	constexpr const Month Feb = date::feb;
	constexpr const Month Mar = date::mar;
	constexpr const Month Apr = date::apr;
	// constexpr const Month May = date::may; Already defined
	constexpr const Month Jun = date::jun;
	constexpr const Month Jul = date::jul;
	constexpr const Month Aug = date::aug;
	constexpr const Month Sep = date::sep;
	constexpr const Month Oct = date::oct;
	constexpr const Month Nov = date::nov;
	constexpr const Month Dec = date::dec;


	/**
	 * Enumerates the days of the week in 7-day calendars.
	 */
	enum class DayOfWeek
	{
		Sunday    = 0,
		Monday    = 1,
		Tuesday   = 2,
		Wednesday = 3,
		Thursday  = 4,
		Friday    = 5,
		Saturday  = 6
	};


	/**
	 * Enumerates the months of the year in 12-month calendars.
	 */
	enum class MonthOfYear
	{
		January = 1,
		February,
		March,
		April,
		May,
		June,
		July,
		August,
		September,
		October,
		November,
		December
	};


#pragma region Timespan
	/**
	 * Implements a time span.
	 *
	 * A time span is the difference between two dates and times. For example, the time span between
	 * 12:00:00 January 1, 2000 and 18:00:00 January 2, 2000 is 30.0 hours. Time spans are measured
	 * in positive or negative ticks depending on whether the difference is measured forward or
	 * backward. Each tick has a resolution of 0.1 microseconds (= 100 nanoseconds).
	 *
	 * In conjunction with the companion class DateTime, time spans can be used to perform date and
	 * time based arithmetic, such as calculating the difference between two dates or adding a
	 * certain amount of time to a given date.
	 *
	 * When initializing time span values from single components, consider using the FromHours,
	 * FromMinutes, FromSeconds, Zero, MinValue and related methods instead of calling the
	 * overloaded constructors as they will make your code easier to read and understand.
	 *
	 * @see DateTime
	 */
	struct PIPE_API Timespan
	{
	private:
#pragma warning(push)
#pragma warning(disable:4251)
		/** The time span value in 100 nanoseconds resolution. */
		DecMicroseconds duration{};
#pragma warning(pop)


	public:
		/** Default constructor (no initialization). */
		Timespan() = default;

		/**
		 * Create and initialize a new time interval with the specified number of ticks.
		 *
		 * For better readability, consider using MinValue, MaxValue and Zero.
		 *
		 * @param duration The number of decimals of microseconds.
		 * @see MaxValue, MinValue, Zero
		 */
		Timespan(DecMicroseconds duration) : duration(duration) {}

		/**
		 * Create and initialize a new time interval with the specified number of hours, minutes and
		 * seconds.
		 *
		 * For better readability, consider using FromHours, FromMinutes and FromSeconds.
		 *
		 * @param Hours The hours component.
		 * @param Minutes The minutes component.
		 * @param Seconds The seconds component.
		 * @see FromHours, FromMinutes, FromSeconds
		 */
		Timespan(i32 Hours, i32 Minutes, i32 Seconds) : duration{}
		{
			Assign(0, Hours, Minutes, Seconds, 0);
		}

		/**
		 * Create and initialize a new time interval with the specified number of days, hours,
		 * minutes and seconds.
		 *
		 * For better readability, consider using FromDays, FromHours, FromMinutes and FromSeconds.
		 *
		 * @param Days The days component.
		 * @param Hours The hours component.
		 * @param Minutes The minutes component.
		 * @param Seconds The seconds component.
		 * @see FromDays, FromHours, FromMinutes, FromSeconds
		 */
		Timespan(i32 Days, i32 Hours, i32 Minutes, i32 Seconds) : duration{}
		{
			Assign(Days, Hours, Minutes, Seconds, 0);
		}

		/**
		 * Create and initialize a new time interval with the specified number of days, hours,
		 * minutes and seconds.
		 *
		 * @param Days The days component.
		 * @param Hours The hours component.
		 * @param Minutes The minutes component.
		 * @param Seconds The seconds component.
		 * @param FractionNano The fractional seconds (in nanosecond resolution).
		 */
		Timespan(i32 Days, i32 Hours, i32 Minutes, i32 Seconds, i32 FractionNano) : duration{}
		{
			Assign(Days, Hours, Minutes, Seconds, FractionNano);
		}

	public:
		/**
		 * Return the result of adding the given time span to this time span.
		 *
		 * @return A time span whose value is the sum of this time span and the given time span.
		 */
		Timespan operator+(const Timespan& Other) const
		{
			return {duration + Other.duration};
		}

		/**
		 * Adds the given time span to this time span.
		 *
		 * @return This time span.
		 */
		Timespan& operator+=(const Timespan& Other)
		{
			duration += Other.duration;
			return *this;
		}

		/**
		 * Return the inverse of this time span.
		 *
		 * The value of this time span must be greater than FTimespan::MinValue(), or else an
		 * overflow will occur.
		 *
		 * @return Inverse of this time span.
		 */
		Timespan operator-() const
		{
			return {-duration};
		}

		/**
		 * Return the result of subtracting the given time span from this time span.
		 *
		 * @param Other The time span to compare with.
		 * @return A time span whose value is the difference of this time span and the given time
		 * span.
		 */
		Timespan operator-(const Timespan& Other) const
		{
			return {duration - Other.duration};
		}

		/**
		 * Subtract the given time span from this time span.
		 *
		 * @param Other The time span to subtract.
		 * @return This time span.
		 */
		Timespan& operator-=(const Timespan& Other)
		{
			duration -= Other.duration;
			return *this;
		}

		/**
		 * Return the result of multiplying the this time span with the given scalar.
		 *
		 * @param Scalar The scalar to multiply with.
		 * @return A time span whose value is the product of this time span and the given scalar.
		 */
		Timespan operator*(double scalar) const
		{
			return Timespan(DecMicroseconds{(i64)(duration.count() * scalar)});
		}

		/**
		 * Multiply this time span with the given scalar.
		 *
		 * @param Scalar The scalar to multiply with.
		 * @return This time span.
		 */
		Timespan& operator*=(double scalar)
		{
			duration = DecMicroseconds{(i64)(duration.count() * scalar)};
			return *this;
		}

		/**
		 * Return the result of dividing the this time span by the given scalar.
		 *
		 * @param Scalar The scalar to divide by.
		 * @return A time span whose value is the quotient of this time span and the given scalar.
		 */
		Timespan operator/(double scalar) const
		{
			return Timespan(DecMicroseconds{(i64)(duration.count() / scalar)});
		}

		/**
		 * Divide this time span by the given scalar.
		 *
		 * @param Scalar The scalar to divide by.
		 * @return This time span.
		 */
		Timespan& operator/=(double scalar)
		{
			duration = DecMicroseconds{(i64)(duration.count() / scalar)};
			return *this;
		}

		/**
		 * Return the result of calculating the modulus of this time span with another time span.
		 *
		 * @param Other The time span to divide by.
		 * @return A time span representing the remainder of the modulus operation.
		 */
		Timespan operator%(const Timespan& Other) const
		{
			return Timespan(duration % Other.duration);
		}

		/**
		 * Calculate this time span modulo another.
		 *
		 * @param Other The time span to divide by.
		 * @return This time span.
		 */
		Timespan& operator%=(const Timespan& Other)
		{
			duration %= Other.duration;
			return *this;
		}

		/**
		 * Compare this time span with the given time span for equality.
		 *
		 * @param Other The time span to compare with.
		 * @return true if the time spans are equal, false otherwise.
		 */
		bool operator==(const Timespan& Other) const
		{
			return (duration == Other.duration);
		}

		/**
		 * Compare this time span with the given time span for inequality.
		 *
		 * @param Other The time span to compare with.
		 * @return true if the time spans are not equal, false otherwise.
		 */
		bool operator!=(const Timespan& Other) const
		{
			return (duration != Other.duration);
		}

		/**
		 * Check whether this time span is greater than the given time span.
		 *
		 * @param Other The time span to compare with.
		 * @return true if this time span is greater, false otherwise.
		 */
		bool operator>(const Timespan& Other) const
		{
			return (duration > Other.duration);
		}

		/**
		 * Check whether this time span is greater than or equal to the given time span.
		 *
		 * @param Other The time span to compare with.
		 * @return true if this time span is greater or equal, false otherwise.
		 */
		bool operator>=(const Timespan& other) const
		{
			return (duration >= other.duration);
		}

		/**
		 * Check whether this time span is less than the given time span.
		 *
		 * @param Other The time span to compare with.
		 * @return true if this time span is less, false otherwise.
		 */
		bool operator<(const Timespan& other) const
		{
			return (duration < other.duration);
		}

		/**
		 * Check whether this time span is less than or equal to the given time span.
		 *
		 * @param Other The time span to compare with.
		 * @return true if this time span is less or equal, false otherwise.
		 */
		bool operator<=(const Timespan& other) const
		{
			return (duration <= other.duration);
		}

	public:
		/**
		 * Get the days component of this time span.
		 *
		 * @return Days component.
		 */
		i32 GetDays() const
		{
			return Chrono::floor<Days>(duration).count();
		}

		/**
		 * Get a time span with the absolute value of this time span.
		 *
		 * This method may overflow the timespan if its value is equal to MinValue.
		 *
		 * @return Duration of this time span.
		 * @see MinValue
		 */
		Timespan GetDuration()
		{
			return {duration.count() >= 0 ? duration : -duration};
		}

		/**
		 * Gets the fractional seconds (in microsecond resolution).
		 *
		 * @return Number of microseconds in fractional part.
		 * @see GetTotalMicroseconds
		 */
		i64 GetFractionMicro() const
		{
			return (Chrono::floor<Microseconds>(duration) - Chrono::floor<Seconds>(duration))
			    .count();
		}

		/**
		 * Gets the fractional seconds (in millisecond resolution).
		 *
		 * @return Number of milliseconds in fractional part.
		 * @see GetTotalMilliseconds
		 */
		i64 GetFractionMilli() const
		{
			return (Chrono::floor<Milliseconds>(duration) - Chrono::floor<Seconds>(duration))
			    .count();
		}

		/**
		 * Gets the fractional seconds (in nanosecond resolution).
		 *
		 * @return Number of nanoseconds in fractional part.
		 */
		i64 GetFractionNano() const
		{
			return (Chrono::floor<Nanoseconds>(duration) - Chrono::floor<Seconds>(duration))
			    .count();
		}

		/**
		 * Gets the fractional ticks (in 100 nanosecond resolution).
		 *
		 * @return Number of ticks in fractional part.
		 */
		i64 GetFractionTicks() const
		{
			return (duration - Chrono::floor<Seconds>(duration)).count();
		}

		/**
		 * Gets the hours component of this time span.
		 *
		 * @return Hours component.
		 * @see GetTotalHours
		 */
		i32 GetHours() const
		{
			return (i32)(Chrono::floor<Hours>(duration) - Chrono::floor<Days>(duration)).count();
		}

		/**
		 * Get the minutes component of this time span.
		 *
		 * @return Minutes component.
		 * @see GetTotalMinutes
		 */
		i32 GetMinutes() const
		{
			return (i32)(Chrono::floor<Minutes>(duration) - Chrono::floor<Hours>(duration)).count();
		}

		/**
		 * Get the seconds component of this time span.
		 *
		 * @return Seconds component.
		 * @see GetTotalSeconds
		 */
		i32 GetSeconds() const
		{
			return (i32)(Chrono::floor<Seconds>(duration) - Chrono::floor<Minutes>(duration))
			    .count();
		}

		/**
		 * Get the number of ticks represented by this time span.
		 *
		 * @return Number of ticks.
		 */
		const DecMicroseconds& GetTime() const
		{
			return duration;
		}


		/**
		 * Get the total number of days represented by this time span.
		 *
		 * @return Number of days.
		 * @see GetDays
		 */
		template<typename PrecisionType = float>
		PrecisionType GetTotalDays() const
		{
			return Chrono::duration<PrecisionType, Days::period>(duration).count();
		}

		/**
		 * Get the total number of hours represented by this time span.
		 *
		 * @return Number of hours.
		 * @see GetHours
		 */
		template<typename PrecisionType = float>
		PrecisionType GetTotalHours() const
		{
			return Chrono::duration<PrecisionType, Hours::period>(duration).count();
		}

		/**
		 * Get the total number of microseconds represented by this time span.
		 *
		 * @return Number of microseconds.
		 * @see GetFractionMicro
		 */
		template<typename PrecisionType = float>
		PrecisionType GetTotalMicroseconds() const
		{
			return Chrono::duration<PrecisionType, Chrono::microseconds::period>(duration).count();
		}

		/**
		 * Get the total number of milliseconds represented by this time span.
		 *
		 * @return Number of milliseconds.
		 * @see GetFractionMilli
		 */
		template<typename PrecisionType = float>
		PrecisionType GetTotalMilliseconds() const
		{
			return Chrono::duration<PrecisionType, Milliseconds::period>(duration).count();
		}

		/**
		 * Get the total number of minutes represented by this time span.
		 *
		 * @return Number of minutes.
		 * @see GetMinutes
		 */
		template<typename PrecisionType = float>
		PrecisionType GetTotalMinutes() const
		{
			return Chrono::duration<PrecisionType, Minutes::period>(duration).count();
		}

		/**
		 * Get the total number of seconds represented by this time span.
		 *
		 * @return Number of seconds.
		 * @see GetSeconds
		 */
		template<typename PrecisionType = float>
		PrecisionType GetTotalSeconds() const
		{
			return Chrono::duration<PrecisionType, Seconds::period>(duration).count();
		}

		/**
		 * Check whether this time span is zero.
		 *
		 * @return true if the time span is zero, false otherwise.
		 * @see Zero
		 */
		bool IsZero() const
		{
			return duration == DecMicroseconds::zero();
		}

		/**
		 * Return the string representation of this time span using a default format.
		 *
		 * The returned string has the following format:
		 *		p[d.]hh:mm:ss.fff
		 *
		 * Note that 'p' is the plus or minus sign, and the date component is
		 * omitted for time spans that are shorter than one day.
		 *
		 * Examples:
		 *      -42.15:11:36.457 (45 days, 15 hours, 11 minutes, 36.457 seconds in the past)
		 *      +42.15:11:36.457 (45 days, 15 hours, 11 minutes, 36.457 seconds in the future)
		 *      +15:11:36.457 (15 hours, 11 minutes, 36.457 seconds in the future)
		 *      +00:11:36.457 (11 minutes, 36.457 seconds in the future)
		 *      +00:00:36.457 (36.457 seconds in the future)
		 *
		 * @return String representation.
		 * @see Parse
		 */
		String ToString() const;

		/**
		 * Convert this time span to its string representation.
		 *
		 * The following formatting codes are available:
		 *		%d - prints the days component
		 *		%D - prints the zero-padded days component (00000000..10675199)
		 *		%h - prints the zero-padded hours component (00..23)
		 *		%m - prints the zero-padded minutes component (00..59)
		 *		%s - prints the zero-padded seconds component (00..59)
		 *		%f - prints the zero-padded fractional seconds (000..999)
		 *		%u - prints the zero-padded fractional seconds (000000..999999)
		 *		%n - prints the zero-padded fractional seconds (000000000..999999999)
		 *
		 * Depending on whether the time span is positive or negative, a plus or minus
		 * sign character will always be added in front of the generated string.
		 *
		 * @param Format The format of the returned string.
		 * @return String representation.
		 * @see Parse
		 */
		String ToString(const TChar* Format) const;

	public:
		/**
		 * Return the maximum time span value.
		 *
		 * The maximum time span value is slightly more than 10,675,199 days.
		 *
		 * @return Maximum time span.
		 * @see MinValue,Zero
		 */
		static Timespan MaxValue()
		{
			return {DecMicroseconds::max()};
		}

		/**
		 * Return the minimum time span value.
		 *
		 * The minimum time span value is slightly less than -10,675,199 days.
		 *
		 * @return Minimum time span.
		 * @see MaxValue, ZeroValue
		 */
		static Timespan MinValue()
		{
			return {DecMicroseconds::min()};
		}

		/**
		 * Convert a string to a time span.
		 *
		 * The string must be in one of the following formats:
		 *    p[d.]hh::mm::ss.fff
		 *    p[d.]hh::mm::ss.uuuuuu
		 *    p[d.]hh::mm::ss.nnnnnnnnn
		 *
		 * Note that 'p' is the plus or minus sign, and the date component may be
		 * omitted for time spans that are shorter than one day.
		 *
		 * @param TimespanString The string to convert.
		 * @param OutTimespan Will contain the parsed time span.
		 * @return true if the string was converted successfully, false otherwise.
		 * @see ToString
		 */
		static bool Parse(const String& TimespanString, Timespan& OutTimespan);

		/**
		 * Return the zero time span value.
		 *
		 * The zero time span value can be used in comparison operations with other time spans.
		 *
		 * @return Zero time span.
		 * @see IsZero, MaxValue, MinValue
		 */
		static Timespan Zero()
		{
			return {DecMicroseconds::zero()};
		}

	protected:
		/**
		 * Assign the specified components to this time span.
		 *
		 * @param Days The days component.
		 * @param Hours The hours component.
		 * @param Minutes The minutes component.
		 * @param Seconds The seconds component.
		 * @param FractionNano The fractional seconds (in nanosecond resolution).
		 */
		void Assign(i32 days, i32 hours, i32 minutes, i32 seconds, i32 fractionNano);

	private:
		friend struct Z_Construct_UScriptStruct_FTimespan_Statics;

	public:
		static Timespan FromHours(i32 hours);
		static Timespan FromMinutes(i32 TzOffsetMinutes);
		static Timespan FromSeconds(i32 seconds);
	};


	/**
	 * Pre-multiply a time span with the given scalar.
	 *
	 * @param Scalar The scalar to pre-multiply with.
	 * @param Timespan The time span to multiply.
	 */
	inline Timespan operator*(float Scalar, const Timespan& Timespan)
	{
		return Timespan.operator*(Scalar);
	}
#pragma endregion Timespan


	/**
	 * Implements a date and time.
	 *
	 * Values of this type represent dates and times between Midnight 00:00:00,
	 * January 1, 0001 and Midnight 23:59:59.9999999, December 31, 9999 in the
	 * Gregorian calendar. Internally, the time values are stored in ticks of 0.1
	 * microseconds (= 100 nanoseconds) since January 1, 0001.
	 *
	 * To retrieve the current local date and time, use the FDateTime.Now() method.
	 * To retrieve the current UTC time, use the FDateTime.UtcNow() method instead.
	 *
	 * This class also provides methods to convert dates and times from and to
	 * string representations, calculate the number of days in a given month and
	 * year, check for leap years and determine the time of day, day of week and
	 * month of year of a given date and time.
	 *
	 * The companion struct FTimespan is provided for enabling date and time based
	 * arithmetic, such as calculating the difference between two dates or adding a
	 * certain amount of time to a given date.
	 *
	 * Ranges of dates and times can be represented by the FDateRange class.
	 *
	 * @see FDateRange
	 * @see FTimespan
	 */
	struct PIPE_API DateTime
	{
	protected:
		/** Holds the days per month in a non-leap year. */
		static const i32 DaysPerMonth[];

		/** Holds the cumulative days per month in a non-leap year. */
		static const i32 DaysToMonth[];

#pragma warning(push)
#pragma warning(disable:4251)
		/** Holds the ticks in 100 nanoseconds resolution since January 1, 0001 A.D. */
		SysTime value{};
#pragma warning(pop)


	public:
		/** Default constructor (no initialization). */
		DateTime() = default;

		/**
		 * Creates and initializes a new instance with the specified number of
		 * ticks.
		 *
		 * @param Ticks The ticks representing the date and time.
		 */
		DateTime(SysTime value) : value{value} {}

		template<typename Precision>
		DateTime(Chrono::time_point<SysClock, Precision> value)
		    : value{Chrono::time_point_cast<DecMicroseconds, SysClock, Precision>(value)}
		{}

		/**
		 * Creates and initializes a new instance with the specified year, month,
		 * day, hour, minute, second and millisecond.
		 *
		 * @param Year The year.
		 * @param Month The month.
		 * @param Day The day.
		 * @param Hour The hour (optional).
		 * @param Minute The minute (optional).
		 * @param Second The second (optional).
		 * @param Millisecond The millisecond (optional).
		 */
		DateTime(i32 Year, i32 Month, i32 Day, i32 Hour = 0, i32 Minute = 0, i32 Second = 0,
		    i32 Millisecond = 0);

	public:
		/**
		 * Returns result of adding the given time span to this date.
		 *
		 * @return A date whose value is the sum of this date and the given time
		 * span.
		 * @see FTimespan
		 */
		DateTime operator+(const Timespan& other) const
		{
			return DateTime(value + other.GetTime());
		}

		/**
		 * Adds the given time span to this date.
		 *
		 * @return This date.
		 * @see FTimespan
		 */
		DateTime& operator+=(const Timespan& Other)
		{
			value += Other.GetTime();
			return *this;
		}

		/**
		 * Returns time span between this date and the given date.
		 *
		 * @return A time span whose value is the difference of this date and the
		 * given date.
		 * @see FTimespan
		 */
		Timespan operator-(const DateTime& other) const
		{
			return Timespan(value - other.value);
		}

		/**
		 * Returns result of subtracting the given time span from this date.
		 *
		 * @return A date whose value is the difference of this date and the given
		 * time span.
		 * @see FTimespan
		 */
		DateTime operator-(const Timespan& Other) const
		{
			return DateTime(value - Other.GetTime());
		}

		/**
		 * Subtracts the given time span from this date.
		 *
		 * @return This date.
		 * @see FTimespan
		 */
		DateTime& operator-=(const Timespan& other)
		{
			value -= DecMicroseconds{other.GetTime()};

			return *this;
		}

		/**
		 * Compares this date with the given date for equality.
		 *
		 * @param other The date to compare with.
		 * @return true if the dates are equal, false otherwise.
		 */
		bool operator==(const DateTime& Other) const
		{
			return (value == Other.value);
		}

		/**
		 * Compares this date with the given date for inequality.
		 *
		 * @param other The date to compare with.
		 * @return true if the dates are not equal, false otherwise.
		 */
		bool operator!=(const DateTime& Other) const
		{
			return (value != Other.value);
		}

		/**
		 * Checks whether this date is greater than the given date.
		 *
		 * @param other The date to compare with.
		 * @return true if this date is greater, false otherwise.
		 */
		bool operator>(const DateTime& Other) const
		{
			return (value > Other.value);
		}

		/**
		 * Checks whether this date is greater than or equal to the date span.
		 *
		 * @param other The date to compare with.
		 * @return true if this date is greater or equal, false otherwise.
		 */
		bool operator>=(const DateTime& Other) const
		{
			return (value >= Other.value);
		}

		/**
		 * Checks whether this date is less than the given date.
		 *
		 * @param other The date to compare with.
		 * @return true if this date is less, false otherwise.
		 */
		bool operator<(const DateTime& Other) const
		{
			return (value < Other.value);
		}

		/**
		 * Checks whether this date is less than or equal to the given date.
		 *
		 * @param other The date to compare with.
		 * @return true if this date is less or equal, false otherwise.
		 */
		bool operator<=(const DateTime& Other) const
		{
			return (value <= Other.value);
		}

	public:
		const SysTime& GetTime() const
		{
			return value;
		}

		i64 GetTicks() const
		{
			return value.time_since_epoch().count();
		}

		/**
		 * Gets the date part of this date.
		 *
		 * The time part is truncated and becomes 00:00:00.000.
		 *
		 * @return A FDateTime object containing the date.
		 */
		DateTime GetDate() const
		{
			return DateTime(Chrono::floor<date::days>(value));
		}

		/**
		 * Gets the date components of this date.
		 *
		 * @param OutYear Will contain the year.
		 * @param OutMonth Will contain the number of the month (1-12).
		 * @param OutDay Will contain the number of the day (1-31).
		 */
		YearMonthDay GetDateComponents() const;

		/**
		 * Gets this date's day part (1 to 31).
		 *
		 * @return Day of the month.
		 * @see GetHour, GetHour12, GetMillisecond, GetMinute, GetMonth, GetSecond,
		 * GetYear
		 */
		u32 GetDay() const;

		/**
		 * Calculates this date's day of the week (Sunday - Saturday).
		 *
		 * @return The week day.
		 * @see GetDayOfYear, GetMonthOfYear, GetTimeOfDay
		 */
		DayOfWeek GetDayOfWeek() const;

		/**
		 * Gets this date's day of the year.
		 *
		 * @return The day of year.
		 * @see GetDayOfWeek, GetMonthOfYear, GetTimeOfDay
		 */
		u32 GetDayOfYear() const;

		/**
		 * Gets this date's hour part in 24-hour clock format (0 to 23).
		 *
		 * @return The hour.
		 * @see GetDay, GetDayOfWeek, GetDayOfYear, GetHour12, GetMillisecond,
		 * GetMinute, GetMonth, GetSecond, GetYear
		 */
		u32 GetHour() const
		{
			return (Chrono::floor<Chrono::hours>(value) - Chrono::floor<date::days>(value)).count();
		}

		/**
		 * Gets this date's hour part in 12-hour clock format (1 to 12).
		 *
		 * @return The hour in AM/PM format.
		 * @see GetDay, GetHour, GetMillisecond, GetMinute, GetMonth, GetSecond,
		 * GetYear
		 */
		u32 GetHour12() const;

		/**
		 * Gets this date's millisecond part (0 to 999).
		 *
		 * @return The millisecond.
		 * @see GetDay, GetHour, GetHour12, GetMinute, GetMonth, GetSecond, GetYear
		 */
		i32 GetMillisecond() const
		{
			return (i32)(Chrono::floor<Chrono::milliseconds>(value)
			             - Chrono::floor<Chrono::seconds>(value))
			    .count();
		}

		/**
		 * Gets this date's minute part (0 to 59).
		 *
		 * @return The minute.
		 * @see GetDay, GetHour, GetHour12, GetMillisecond, GetMonth, GetSecond,
		 * GetYear
		 */
		i32 GetMinute() const
		{
			return (Chrono::floor<Chrono::minutes>(value) - Chrono::floor<Chrono::hours>(value))
			    .count();
		}

		/**
		 * Gets this date's the month part (1 to 12).
		 *
		 * @return The month.
		 * @see GetDay, GetHour, GetHour12, GetMillisecond, GetMinute, GetSecond,
		 * GetYear
		 */
		u32 GetMonth() const;

		/**
		 * Gets the date's month of the year (January to December).
		 *
		 * @return Month of year.
		 * @see GetDayOfWeek, GetDayOfYear, GetTimeOfDay
		 */
		MonthOfYear GetMonthOfYear() const
		{
			return static_cast<MonthOfYear>(GetMonth());
		}

		/**
		 * Gets this date's second part.
		 *
		 * @return The second.
		 * @see GetDay, GetHour, GetHour12, GetMillisecond, GetMinute, GetMonth,
		 * GetYear
		 */
		i32 GetSecond() const
		{
			return (
			    i32)(Chrono::floor<Chrono::seconds>(value) - Chrono::floor<Chrono::minutes>(value))
			    .count();
		}

		/**
		 * Gets the time elapsed since midnight of this date.
		 *
		 * @param Time of day since midnight.
		 * @see GetDayOfWeek, GetDayOfYear, GetMonthOfYear
		 */
		Timespan GetTimeOfDay() const
		{
			return Timespan(value - Chrono::floor<date::days>(value));
		}

		/**
		 * Gets this date's year part.
		 *
		 * @return The year.
		 * @see GetDay, GetHour, GetHour12, GetMillisecond, GetMinute, GetMonth,
		 * GetSecond
		 */
		i32 GetYear() const;

		/**
		 * Gets whether this date's time is in the afternoon.
		 *
		 * @param true if it is in the afternoon, false otherwise.
		 * @see IsMorning
		 */
		bool IsAfternoon() const
		{
			return (GetHour() >= 12);
		}

		/**
		 * Gets whether this date's time is in the morning.
		 *
		 * @param true if it is in the morning, false otherwise.
		 * @see IsAfternoon
		 */
		bool IsMorning() const
		{
			return (GetHour() < 12);
		}

		void Read(class Reader& ct);
		void Write(class Writer& ct) const;

		/**
		 * Returns the RFC 1123 string representation of the FDateTime.
		 *
		 * The resulting string assumes that the FDateTime is in UTC.
		 *
		 * @return String representation.
		 * @see ParseHttpDate, ToIso8601, ToString
		 */
		String ToHttpDate() const;

		/**
		 * Returns the ISO-8601 string representation of the FDateTime.
		 *
		 * The resulting string assumes that the FDateTime is in UTC.
		 *
		 * @return String representation.
		 * @see ParseIso8601, ToHttpDate, ToString
		 */
		String ToIso8601() const;

		/**
		 * Returns the string representation of this date using a default format.
		 *
		 * The returned string has the following format:
		 *		yyyy.mm.dd-hh.mm.ss
		 *
		 * @return String representation.
		 * @see Parse, ToIso8601
		 */
		String ToString() const;

		/**
		 * Returns the string representation of this date.
		 *
		 * @param Format The format of the returned string.
		 * @return String representation.
		 * @see Parse, ToIso8601
		 */
		String ToString(const TChar* Format) const;

		/**
		 * Returns this date as the number of seconds since the Unix Epoch (January
		 * 1st of 1970).
		 *
		 * @return Time of day.
		 * @see FromUnixTimestamp
		 */
		i64 ToUnixTimestamp() const
		{
			return Chrono::floor<Chrono::seconds>(value).time_since_epoch().count();
		}

		DateTime ToLocal() const;
		DateTime ToUTC() const;

	public:
		/**
		 * Gets the number of days in the year and month.
		 *
		 * @param Year The year.
		 * @param Month The month.
		 * @return The number of days
		 * @see DaysInYear
		 */
		static i32 DaysInMonth(i32 Year, i32 Month);

		/**
		 * Gets the number of days in the given year.
		 *
		 * @param Year The year.
		 * @return The number of days.
		 * @see DaysInMonth
		 */
		static i32 DaysInYear(i32 Year);


		/**
		 * Returns the date from Unix time (seconds from midnight 1970-01-01)
		 *
		 * @param UnixTime Unix time (seconds from midnight 1970-01-01)
		 * @return Gregorian date and time.
		 * @see ToUnixTimestamp
		 */
		static DateTime FromUnixTimestamp(i64 UnixTime)
		{
			return DateTime(1970, 1, 1) + Timespan(Chrono::seconds{UnixTime});
		}

		/**
		 * Checks whether the given year is a leap year.
		 *
		 * A leap year is a year containing one additional day in order to keep the
		 * calendar synchronized with the astronomical year. All years divisible by
		 * 4, but not divisible by 100 - except if they are also divisible by 400 -
		 * are leap years.
		 *
		 * @param Year The year to check.
		 * @return true if the year is a leap year, false otherwise.
		 */
		static bool IsLeapYear(i32 Year);

		/**
		 * Returns the maximum date value.
		 *
		 * The maximum date value is December 31, 9999, 23:59:59.9999999.
		 *
		 * @see MinValue
		 */
		static DateTime MaxValue()
		{
			return {SysTime::max()};
		}

		/**
		 * Returns the minimum date value.
		 *
		 * The minimum date value is January 1, 0001, 00:00:00.0.
		 *
		 * @see MaxValue
		 */
		static DateTime MinValue()
		{
			return {SysTime::min()};
		}

		/**
		 * Gets the local date and time on this computer.
		 *
		 * This method takes into account the local computer's time zone and
		 * daylight saving settings. For time zone independent time comparisons, and
		 * when comparing times between different computers, please use UtcNow()
		 * instead.
		 *
		 * @return Current date and time.
		 * @see Today, UtcNow
		 */
		static DateTime Now();

		/**
		 * Gets the current UTC date and time.
		 *
		 * @return Current date and time.
		 * @see Today, Now
		 */
		static DateTime UtcNow();

		/**
		 * Converts a string to a date and time.
		 *
		 * Currently, the string must be in the format written by either
		 * FDateTime.ToString() or FTimeStamp.TimestampToFString(). Other formats
		 * are not supported at this time.
		 *
		 * @param DateTimeString The string to convert.
		 * @param OutDateTime Will contain the parsed date and time.
		 * @return true if the string was converted successfully, false otherwise.
		 * @see ParseHttpDate, ParseIso8601, ToString
		 */
		static bool Parse(const String& DateTimeString, DateTime& OutDateTime);

		/**
		 * Parses a date string in HTTP-date format (rfc1123-date | rfc850-date |
		 * asctime-date)
		 * https://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3.1
		 *
		 * HTTP-date    = rfc1123-date | rfc850-date | asctime-date
		 * rfc1123-date = wkday "," SP date1 SP time SP "GMT"
		 * rfc850-date  = weekday "," SP date2 SP time SP "GMT"
		 * asctime-date = wkday SP date3 SP time SP 4DIGIT
		 * date1        = 2DIGIT SP month SP 4DIGIT ; day month year (e.g., 02 Jun
		 * 1982) date2        = 2DIGIT "-" month "-" 2DIGIT ; day-month-year (e.g.,
		 * 02-Jun-82) date3        = month SP (2DIGIT | (SP 1DIGIT)) ; month day
		 * (e.g., Jun  2) time         = 2DIGIT ":" 2DIGIT ":" 2DIGIT ; 00:00:00 -
		 * 23:59:59 wkday        = "Mon" | "Tue" | "Wed" | "Thu" | "Fri" | "Sat" |
		 * "Sun" weekday      = "Monday" | "Tuesday" | "Wednesday" | "Thursday" |
		 * "Friday" | "Saturday" | "Sunday" month        = "Jan" | "Feb" | "Mar" |
		 * "Apr" | "May" | "Jun" | "Jul" | "Aug" | "Sep" | "Oct" | "Nov" | "Dec"
		 *
		 * @param HttpDate The string to be parsed
		 * @param OutDateTime FDateTime object (assumes UTC) corresponding to the
		 * input string.
		 * @return true if the string was converted successfully, false otherwise.
		 * @see Parse, ToHttpDate, ParseIso8601
		 */
		static bool ParseHttpDate(const String& HttpDate, DateTime& OutDateTime);

		/**
		 * Parses a date string in ISO-8601 format.
		 *
		 * @param DateTimeString The string to be parsed
		 * @param OutDateTime FDateTime object (in UTC) corresponding to the input
		 * string (which may have been in any timezone).
		 * @return true if the string was converted successfully, false otherwise.
		 * @see Parse, ParseHttpDate, ToIso8601
		 */
		static bool ParseIso8601(const TChar* DateTimeString, DateTime& OutDateTime);

		/**
		 * Gets the local date on this computer.
		 *
		 * The time component is set to 00:00:00
		 *
		 * @return Current date.
		 * @see Now, UtcNow
		 */
		static DateTime Today()
		{
			return Now().GetDate();
		}

		/**
		 * Validates the given components of a date and time value.
		 *
		 * The allow ranges for the components are:
		 *		Year: 1 - 9999
		 *		Month: 1 - 12
		 *		Day: 1 - DaysInMonth(Month)
		 *		Hour: 0 - 23
		 *		Minute: 0 - 59
		 *		Second: 0 - 59
		 *		Millisecond: 0 - 999
		 *
		 * @return true if the components are valid, false otherwise.
		 */
		static bool Validate(
		    i32 Year, i32 Month, i32 Day, i32 Hour, i32 Minute, i32 Second, i32 Millisecond);

		template<typename Clock, typename Precision>
		static DateTime CastClock(Chrono::time_point<Clock, Precision> time)
		{
			return {InternalCastClock<DecMicroseconds, Precision, SysClock, Clock>(time)};
		}

	protected:
		/**
		 * Cast to a different type of clock.
		 * This operation may contains small precision errors.
		 */
		template<typename DstDuration, typename SrcDuration, typename DstClock, typename SrcClock,
		    typename DstTime = Chrono::time_point<DstClock, DstDuration>,
		    typename SrcTime = Chrono::time_point<SrcClock, SrcDuration>>
		static DstTime InternalCastClock(const SrcTime tp,
		    const SrcDuration tolerance = DecMicroseconds{1}, const i32 limit = 10)
		{
			assert(limit > 0);
			auto itercnt = 0;
			auto src_now = SrcTime{};
			auto dst_now = DstTime{};

			SrcDuration epsilon = SrcDuration::max();
			do
			{
				const auto src_before  = SrcClock::now();
				const auto dst_between = DstClock::now();
				const auto src_after   = SrcClock::now();
				const auto src_diff    = src_after - src_before;
				const auto delta       = Chrono::abs(src_diff);
				if (delta < epsilon)
				{
					src_now = src_before + src_diff / 2;
					dst_now = dst_between;
					epsilon = delta;
				}
				if (++itercnt >= limit)
					break;
			} while (epsilon > tolerance);

			return dst_now + (tp - src_now);
		}

	private:
		friend struct Z_Construct_UScriptStruct_FDateTime_Statics;
	};


	struct PIPE_API FrameTime
	{
	protected:
		DateTime previousTime;
		DateTime currentTime = DateTime::Now();

		float realDeltaTime = 0.f;
		float deltaTime     = 0.f;
		float timeDilation  = 1.f;

		// Value of 1/FPS_CAP
		float minFrameTime = 0.f;    // Uncapped


	public:
		FrameTime() = default;

		// Call before tick
		void PreTick();

		// Call after tick
		void PostTick();

		void SetFPSCap(u32 maxFPS)
		{
			minFrameTime = 1.f / maxFPS;
		}

		void SetTimeDilation(float newTimeDilation)
		{
			timeDilation = newTimeDilation;
		}

		float GetDeltaTime() const
		{
			return deltaTime;
		}
	};
}    // namespace p
