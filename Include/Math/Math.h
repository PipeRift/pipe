// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Core/Macros.h"
#include "Core/Platform.h"
#include "Core/Utility.h"
#include "TypeTraits.h"

#include <cmath>


namespace p::math
{
	constexpr float PI{3.14159265358979323846f};
	constexpr float HALF_PI  = PI * 0.5f;
	constexpr float INV_PI   = 1.f / PI;
	constexpr float RADTODEG = 180.f / PI;
	constexpr float DEGTORAD = PI / 180.f;

	constexpr float SMALL_NUMBER{1.e-4f};
	constexpr float SMALLER_NUMBER{0.00001f};
	constexpr float VERY_SMALL_NUMBER{1.e-8f};
	constexpr float BIG_NUMBER{3.4e+38f};
	constexpr float EULER{2.71828182845904523536f};


	template<typename Type>
	static constexpr Type Max(Type a, Type b)
	{
		return a >= b ? a : b;
	}

	template<typename Type>
	static constexpr Type Min(Type a, Type b)
	{
		return a <= b ? a : b;
	}

	template<typename Type>
	static constexpr Type Clamp(Type a, Type min, Type max)
	{
		return Max(min, Min(a, max));
	}

	/** Computes absolute value in a generic way */
	template<typename Type>
	static constexpr Type Abs(const Type a)
	{
		return (a >= (Type)0) ? a : -a;
	}

	/** Returns 1, 0, or -1 depending on relation of Type to 0 */
	template<typename Type>
	static constexpr Type Sign(const Type a)
	{
		return (a > (Type)0) ? (Type)1 : ((a < (Type)0) ? (Type)-1 : (Type)0);
	}


	/** Returns a random integer between 0 and RAND_MAX, inclusive */
	inline CORE_API i32 Rand()
	{
		return rand();
	}

	/** Returns a random float between 0 and 1, inclusive. */
	inline CORE_API float Rand01()
	{
		return Rand() / (float)RAND_MAX;
	}


	template<typename T>
	static constexpr bool IsPosInf(const T x)
	{
		return x == std::numeric_limits<T>::infinity();
	}

	template<typename T>
	static constexpr bool IsNegInf(const T x)
	{
		return x == -std::numeric_limits<T>::infinity();
	}

	template<typename T>
	static constexpr bool IsInf(const T x)
	{
		return IsNegInf(x) || IsPosInf(x);
	}

	template<typename T>
	static constexpr bool IsNAN(const T x) noexcept
	{
		return x != x;
	}


	template<FloatingPoint T>
	static constexpr T Floor(T v)
	{
		if (std::is_constant_evaluated())
		{
			if (IsNAN(v))
			{
				return v;
			}
			else if (IsInf(v))
			{
				return v;
			}
			else if (std::numeric_limits<T>::epsilon() > Abs(v))
			{
				return v;
			}
			const i64 i = static_cast<i64>(v);
			return T(v < i ? i - 1 : i);
		}
		return std::floor(v);
	}

	CORE_API constexpr i32 FloorToI32(float f)
	{
		return i32(Floor(f));
	}

	CORE_API constexpr i64 FloorToI64(double f)
	{
		return i64(Floor(f));
	}


	template<FloatingPoint T>
	static constexpr T Ceil(T v)
	{
		if (std::is_constant_evaluated())
		{
			if (IsNAN(v))
			{
				return v;
			}
			else if (IsInf(v))
			{
				return v;
			}
			else if (std::numeric_limits<T>::epsilon() > Abs(v))
			{
				return v;
			}
			const i64 i = static_cast<i64>(v);
			return T(v > i ? i + 1 : i);
		}
		return std::ceil(v);
	}
	CORE_API constexpr i32 CeilToI32(float f)
	{
		return i32(Ceil(f));
	}
	CORE_API constexpr i64 CeilToI64(double f)
	{
		return i64(Ceil(f));
	}


	inline CORE_API float Round(float f)
	{
		return std::round(f);
	}
	inline CORE_API double Round(double f)
	{
		return std::round(f);
	}
	inline CORE_API i32 RoundToInt(float f)
	{
		return i32(Round(f));
	}

	/**
	 * Converts a floating point number to an integer which is further from zero, "larger" in
	 * absolute value: 0.1 becomes 1, -0.1 becomes -1
	 * @param f Floating point value to convert
	 * @return The rounded integer
	 */
	inline CORE_API float RoundFromZero(float f)
	{
		return (f < 0.0f) ? Floor(f) : Ceil(f);
	}

	inline CORE_API double RoundFromZero(double d)
	{
		return (d < 0.0) ? Floor(d) : Ceil(d);
	}

	/**
	 * Converts a floating point number to an integer which is closer to zero, "smaller" in
	 * absolute value: 0.1 becomes 0, -0.1 becomes 0
	 * @param v Floating point value to convert
	 * @return The rounded integer
	 */
	inline CORE_API float RoundToZero(float f)
	{
		return (f < 0.0f) ? Ceil(f) : Floor(f);
	}

	inline CORE_API double RoundToZero(double d)
	{
		return (d < 0.0) ? Ceil(d) : Floor(d);
	}

	/**
	 * Converts a floating point number to an integer which is more negative: 0.1 becomes 0,
	 * -0.1 becomes -1
	 * @param F		Floating point value to convert
	 * @return		The rounded integer
	 */
	inline CORE_API float RoundToNegativeInfinity(float f)
	{
		return Floor(f);
	}

	inline CORE_API double RoundToNegativeInfinity(double d)
	{
		return Floor(d);
	}

	/**
	 * Converts a floating point number to an integer which is more positive: 0.1 becomes 1,
	 * -0.1 becomes 0
	 * @param F		Floating point value to convert
	 * @return		The rounded integer
	 */
	inline CORE_API float RoundToPositiveInfinity(float f)
	{
		return Ceil(f);
	}

	inline CORE_API double RoundToPositiveInfinity(double d)
	{
		return Ceil(d);
	}


	template<typename T>
	static T Sqrt(T val)
	{
		return std::sqrt(val);
	}

	inline CORE_API float InvSqrt(float x)
	{
		return 1.f / Sqrt(x);
	}

	template<typename T>
	static constexpr T Square(T val)
	{
		return val * val;
	}

	template<SignedIntegral T>
	CORE_API constexpr T Pow(T value, u32 power)
	{
		if (power == 0)
			return value >= 0 ? 1 : -1;

		T result = value;
		for (u32 i = 1; i < power; ++i)
			result *= value;
		return result;
	}
	template<UnsignedIntegral T>
	CORE_API constexpr T Pow(T value, u32 power)
	{
		if (power == 0)
			return 1u;

		T result = value;
		for (u32 i = 1; i < power; ++i)
			result *= value;
		return result;
	}

	template<FloatingPoint V, Number P>
	static constexpr V Pow(V value, P power)
	{
		if (std::is_constant_evaluated() && Integral<P>)
		{
			if (power == 0)
				return V(1);

			V result = value;
			for (P i = 1; i < power; ++i)
				result *= value;
			return result;
		}
		return std::pow(value, power);
	}


	// Same as Max but with N arguments
	template<typename Type, typename... Args>
	static constexpr Type Max(Type a, Type b, Args... args)
	{
		return Max(a, Max(b, Forward<Args>(args)...));
	}

	// Same as Min but with N arguments
	template<typename Type, typename... Args>
	static constexpr Type Min(Type a, Type b, Args... args)
	{
		return Min(a, Min(b, Forward<Args>(args)...));
	}


	/*template< class T, class U, EnableIfPassByValue(T)>
	static constexpr T Lerp(const T A, const T B, const U Alpha)
	{
	    return (T)(A + Alpha * (B - A));
	}*/

	template<class T, class U /*, EnableIfNotPassByValue(T)*/>
	static constexpr T Lerp(const T& a, const T& b, const U& alpha)
	{
		return T(a + (b - a) * alpha);
	}

	template<FloatingPoint Type>
	static constexpr float Mod(Type a, Type b)
	{
		return a - b * Floor(a / b);
	}

	template<SignedIntegral Type>
	static constexpr Type Mod(Type a, Type b)
	{
		return ((a % b) + b) % b;
	}

	template<UnsignedIntegral Type>
	static constexpr Type Mod(Type a, Type b)
	{
		return a - b * (a / b);
	}

	CORE_API void SinCos(float value, float& outSin, float& outCos);

	CORE_API float Atan2(float Y, float X);

	/**
	 * Computes the ASin of a scalar value.
	 *
	 * @param Value  input angle
	 * @return ASin of Value
	 */
	inline CORE_API float FastAsin(float Value)
	{
		// Note:  We use FASTASIN_HALF_PI instead of HALF_PI inside of FastASin(), since it was
		// the value that accompanied the minimax coefficients below. It is important to use
		// exactly the same value in all places inside this function to ensure that
		// FastASin(0.0f) == 0.0f. For comparison:
		//		Half PI				== 1.57079632679f == 0x3fC90FDB
		//		FastAsin Half PI	== 1.5707963050f  == 0x3fC90FDA
		static constexpr float fastAsinHalfPI = 1.5707963050f;

		// Clamp input to [-1,1].
		bool nonnegative = (Value >= 0.0f);
		float x          = math::Abs(Value);
		float omx        = 1.0f - x;
		if (omx < 0.0f)
		{
			omx = 0.0f;
		}
		float root = math::Sqrt(omx);
		// 7-degree minimax approximation
		// clang-format off
			float result = ((((((-0.0012624911f * x + 0.0066700901f)
				* x - 0.0170881256f) * x + 0.0308918810f)
				* x - 0.0501743046f) * x + 0.0889789874f)
				* x - 0.2145988016f) * x + fastAsinHalfPI;
		// clang-format on

		result *= root;    // acos(|x|)
		// acos(x) = pi - acos(-x) when x < 0, asin(x) = pi/2 - acos(x)
		return (nonnegative ? fastAsinHalfPI - result : result - fastAsinHalfPI);
	}

	inline CORE_API bool NearlyEqual(float a, float b, float tolerance = SMALL_NUMBER)
	{
		return Abs(b - a) <= tolerance;
	}

	inline CORE_API float Log(float k)
	{
		return std::log(k);
	}
	inline CORE_API float Log(float k, float base)
	{
		return Log(k) / Log(base);
	}
	inline CORE_API double Log(double k)
	{
		return std::log(k);
	}
	inline CORE_API double Log(double k, double base)
	{
		return Log(k) / Log(base);
	}
	template<Integral T>
	static double Log(T k)
	{
		return std::log(k);
	}

	inline CORE_API float Log2(float k)
	{
		return std::log2f(k);
	}
	inline CORE_API double Log2(double k)
	{
		return std::log2(k);
	}
	template<Integral T>
	static double Log2(T k)
	{
		return std::log2(double(k));
	}

	inline CORE_API const float Exp2(const float k)
	{
		return std::exp2(k);
	}
	inline CORE_API const double Exp2(const double k)
	{
		return std::exp2(k);
	}
	template<Integral T>
	static const double Exp2(T k)
	{
		return std::exp2(k);
	}

	template<typename T>
	static bool IsPowerOfTwo(T value)
	{
		return (value & (value - 1)) == T(0);
	}

	CORE_API float ClampAngle(float a);

	CORE_API float NormalizeAngle(float a);

	CORE_API float ClampAngle(float a, float min, float max);
}    // namespace p::math
