// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "Platform/Macros.h"
#include "Platform/Platform.h"
#include "TypeTraits.h"

#include <cmath>


namespace Rift
{
	struct CORE_API Math
	{
		static constexpr float PI{3.14159265358979323846f};
		static constexpr float HALF_PI  = PI * 0.5f;
		static constexpr float INV_PI   = 1 / PI;
		static constexpr float RADTODEG = 180.f / PI;
		static constexpr float DEGTORAD = PI / 180.f;

		static constexpr float SMALL_NUMBER{1.e-4f};
		static constexpr float SMALLER_NUMBER{0.00001f};
		static constexpr float VERY_SMALL_NUMBER{1.e-8f};
		static constexpr float BIG_NUMBER{3.4e+38f};
		static constexpr float EULER{2.71828182845904523536f};


		template <typename Type>
		static constexpr Type Max(Type a, Type b)
		{
			return a >= b ? a : b;
		}

		template <typename Type>
		static constexpr Type Min(Type a, Type b)
		{
			return a <= b ? a : b;
		}

		template <typename Type>
		static constexpr Type Clamp(Type a, Type min, Type max)
		{
			return Max(min, Min(a, max));
		}

		static float ClampAngle(float a)
		{
			const float cAngle = Mod(a, 360.f);             //(-360,360)
			return cAngle + float(cAngle < 0.f) * 360.f;    //[0, 360)
		}

		static float NormalizeAngle(float a)
		{
			a = ClampAngle(a);    //[0,360)

			if (a > 180.f)
				a -= 360.f;
			return a;    //(-180, 180]
		}

		static float ClampAngle(float a, float min, float max)
		{
			const float maxDelta        = ClampAngle(max - min) * 0.5f;       // 0..180
			const float rangeCenter     = ClampAngle(min + maxDelta);         // 0..360
			const float deltaFromCenter = NormalizeAngle(a - rangeCenter);    // -180..180

			// maybe clamp to nearest edge
			if (deltaFromCenter > maxDelta)
			{
				return NormalizeAngle(rangeCenter + maxDelta);
			}
			else if (deltaFromCenter < -maxDelta)
			{
				return NormalizeAngle(rangeCenter - maxDelta);
			}
			return NormalizeAngle(a);    // Already in range
		}

		/** Computes absolute value in a generic way */
		template <typename Type>
		static constexpr Type Abs(const Type a)
		{
			return (a >= (Type) 0) ? a : -a;
		}

		/** Returns 1, 0, or -1 depending on relation of Type to 0 */
		template <typename Type>
		static constexpr Type Sign(const Type a)
		{
			return (a > (Type) 0) ? (Type) 1 : ((a < (Type) 0) ? (Type) -1 : (Type) 0);
		}


		/** Returns a random integer between 0 and RAND_MAX, inclusive */
		static i32 Rand()
		{
			return rand();
		}

		/** Returns a random float between 0 and 1, inclusive. */
		static float Rand01()
		{
			return Rand() / (float) RAND_MAX;
		}

		static i32 RoundToInt(float f)
		{
			return (i32) std::round(f);
		}

		/**
		 * Converts a floating point number to an integer which is further from zero, "larger" in
		 * absolute value: 0.1 becomes 1, -0.1 becomes -1
		 * @param F		Floating point value to convert
		 * @return		The rounded integer
		 */
		static float RoundFromZero(float F)
		{
			return (F < 0.0f) ? FloorToFloat(F) : CeilToFloat(F);
		}

		static double RoundFromZero(double F)
		{
			return (F < 0.0) ? FloorToDouble(F) : CeilToDouble(F);
		}

		/**
		 * Converts a floating point number to an integer which is closer to zero, "smaller" in
		 * absolute value: 0.1 becomes 0, -0.1 becomes 0
		 * @param F		Floating point value to convert
		 * @return		The rounded integer
		 */
		static float RoundToZero(float f)
		{
			return (f < 0.0f) ? CeilToFloat(f) : FloorToFloat(f);
		}

		static double RoundToZero(double d)
		{
			return (d < 0.0) ? CeilToDouble(d) : FloorToDouble(d);
		}

		/**
		 * Converts a floating point number to an integer which is more negative: 0.1 becomes 0,
		 * -0.1 becomes -1
		 * @param F		Floating point value to convert
		 * @return		The rounded integer
		 */
		static float RoundToNegativeInfinity(float f)
		{
			return FloorToFloat(f);
		}

		static double RoundToNegativeInfinity(double d)
		{
			return FloorToDouble(d);
		}

		/**
		 * Converts a floating point number to an integer which is more positive: 0.1 becomes 1,
		 * -0.1 becomes 0
		 * @param F		Floating point value to convert
		 * @return		The rounded integer
		 */
		static float RoundToPositiveInfinity(float f)
		{
			return CeilToFloat(f);
		}

		static double RoundToPositiveInfinity(double d)
		{
			return CeilToDouble(d);
		}

		static i32 FloorToI32(float f)
		{
			return i32(FloorToFloat(f));
		}

		static i64 FloorToI64(double f)
		{
			return i64(FloorToDouble(f));
		}

		static float FloorToFloat(float f)
		{
			return std::floor(f);
		}

		static double FloorToDouble(double d)
		{
			return std::floor(d);
		}

		static i32 CeilToI32(float f)
		{
			return i32(CeilToFloat(f));
		}

		static i64 CeilToI64(double f)
		{
			return i64(CeilToDouble(f));
		}

		static float CeilToFloat(float f)
		{
			return std::ceil(f);
		}

		static double CeilToDouble(double d)
		{
			return std::ceil(d);
		}

		template <typename T>
		static T Sqrt(T val)
		{
			return std::sqrt(val);
		}

		static float InvSqrt(float x)
		{
			return 1.f / Sqrt(x);
		}

		template <typename T>
		static constexpr T Square(T val)
		{
			return val * val;
		}

		static constexpr i32 Pow(i32 value, u32 power);
		static constexpr i64 Pow(i64 value, u32 power);
		static constexpr u32 Pow(u32 value, u32 power);
		static constexpr u64 Pow(u64 value, u32 power);

		template <FloatingPoint V, Number P>
		static V Math::Pow(V value, P power)
		{
			return std::pow(value, power);
		}

		// Same as Max but with N arguments
		template <typename Type, typename... Args>
		static constexpr Type Max(Type a, Type b, Args... args)
		{
			return Max(a, Max(b, std::forward<Args>(args)...));
		}

		// Same as Min but with N arguments
		template <typename Type, typename... Args>
		static constexpr Type Min(Type a, Type b, Args... args)
		{
			return Min(a, Min(b, std::forward<Args>(args)...));
		}


		/*template< class T, class U, EnableIfPassByValue(T)>
		static constexpr T Lerp(const T A, const T B, const U Alpha)
		{
		    return (T)(A + Alpha * (B - A));
		}*/

		template <class T, class U /*, EnableIfNotPassByValue(T)*/>
		static constexpr T Lerp(const T& a, const T& b, const U& alpha)
		{
			return T(a + (b - a) * alpha);
		}

		static float Mod(float a, float b)
		{
			return std::fmod(a, b);
		}

		// Module for integer types only
		template <typename Type>
		static constexpr auto Mod(Type a, Type b)
		    -> decltype(EnableIf<std::is_integral_v<Type>, Type>::type)
		{
			return a % b;
		}

		static void SinCos(float* scalarSin, float* scalarCos, float value);

		static float Atan2(float Y, float X);

		// Note:  We use FASTASIN_HALF_PI instead of HALF_PI inside of FastASin(), since it was the
		// value that accompanied the minimax coefficients below. It is important to use exactly the
		// same value in all places inside this function to ensure that FastASin(0.0f) == 0.0f. For
		// comparison:
		//		HALF_PI				== 1.57079632679f == 0x3fC90FDB
		//		FASTASIN_HALF_PI	== 1.5707963050f  == 0x3fC90FDA
#define FASTASIN_HALF_PI (1.5707963050f)
		/**
		 * Computes the ASin of a scalar value.
		 *
		 * @param Value  input angle
		 * @return ASin of Value
		 */
		static float FastAsin(float Value)
		{
			// Clamp input to [-1,1].
			bool nonnegative = (Value >= 0.0f);
			float x          = Math::Abs(Value);
			float omx        = 1.0f - x;
			if (omx < 0.0f)
			{
				omx = 0.0f;
			}
			float root = Math::Sqrt(omx);
			// 7-degree minimax approximation
			float result = ((((((-0.0012624911f * x + 0.0066700901f) * x - 0.0170881256f) * x +
			                      0.0308918810f) *
			                         x -
			                     0.0501743046f) *
			                        x +
			                    0.0889789874f) *
			                       x -
			                   0.2145988016f) *
			                   x +
			               FASTASIN_HALF_PI;
			result *= root;    // acos(|x|)
			// acos(x) = pi - acos(-x) when x < 0, asin(x) = pi/2 - acos(x)
			return (nonnegative ? FASTASIN_HALF_PI - result : result - FASTASIN_HALF_PI);
		}
#undef FASTASIN_HALF_PI

		static bool NearlyEqual(float a, float b, float tolerance = SMALL_NUMBER)
		{
			return Abs(b - a) <= tolerance;
		}

		static const float Log(const float k)
		{
			return std::log(k);
		}
		static const float Log(const float k, float base)
		{
			return Log(k) / Log(base);
		}

		static const double Log(const double k)
		{
			return std::log(k);
		}
		static const double Log(const double k, double base)
		{
			return Log(k) / Log(base);
		}

		template <Integral T>
		static double Log(const T k)
		{
			return log(k);
		}

		static const float Exp2(const float k)
		{
			return std::exp2(k);
		}
		static const double Exp2(const double k)
		{
			return std::exp2(k);
		}
		template <Integral T>
		static const double Exp2(T k)
		{
			return std::exp2(k);
		}

		template <typename T>
		static bool IsPowerOfTwo(T value)
		{
			return (value & (value - 1)) == T(0);
		}
	};
}    // namespace Rift
