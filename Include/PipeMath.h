// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Fwd/PipeSerialize.h"
#include "Pipe/Core/Limits.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Core/Utility.h"
#include "PipePlatform.h"

#include <cmath>


namespace p
{
	constexpr float pi       = 3.14159265358979323846f;
	constexpr float halfPi   = pi * 0.5f;
	constexpr float invPi    = 1.f / pi;
	constexpr float radToDeg = 180.f / pi;
	constexpr float degToRad = pi / 180.f;

	constexpr float smallNumber     = 1.e-4f;
	constexpr float verySmallNumber = 1.e-8f;
	constexpr float bigNumber       = 3.4e+38f;
	constexpr float euler           = 2.71828182845904523536f;


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
	static constexpr Type* Max(Type* values, u32 count)
	{
		if (!values || count <= 0)
		{
			return nullptr;
		}
		Type* max = values;
		for (u32 i = 1; i < count; ++i)
		{
			Type* v = values + i;
			if (*v > *max)
			{
				max = v;
			}
		}
		return max;
	}

	template<typename Type>
	static constexpr Type* Min(Type* values, u32 count)
	{
		if (!values || count <= 0)
		{
			return nullptr;
		}
		Type* min = values;
		for (u32 i = 1; i < count; ++i)
		{
			Type* v = values + i;
			if (*v < *min)
			{
				min = v;
			}
		}
		return min;
	}

	template<typename Type>
	static constexpr std::pair<Type*, Type*> MinMax(Type* values, u32 count)
	{
		if (!values || count <= 0)
		{
			return {nullptr, nullptr};
		}
		Type* min = values;
		Type* max = values;
		for (u32 i = 1; i < count; ++i)
		{
			Type* v = values + i;
			if (*v < *min)
			{
				min = v;
			}
			if (*v > *max)
			{
				max = v;
			}
		}
		return {min, max};
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
	inline P_API i32 Rand()
	{
		return rand();
	}

	/** Returns a random float between 0 and 1, inclusive. */
	inline P_API float Rand01()
	{
		return (float)Rand() / (float)RAND_MAX;
	}

	inline P_API float Random(float min, float max)
	{
		return min + (Rand01() * (max - min));
	}

	inline P_API i32 Random(i32 min, i32 max)
	{
		return min + (Rand01() * (max - min));
	}


	template<typename T>
	static constexpr bool IsPosInf(const T x)
	{
		return x == Limits<T>::Infinity();
	}

	template<typename T>
	static constexpr bool IsNegInf(const T x)
	{
		return x == -Limits<T>::Infinity();
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
			else if (Limits<T>::Epsilon() > Abs(v))
			{
				return v;
			}
			const i64 i = static_cast<i64>(v);
			return T(v < i ? i - 1 : i);
		}
		else
		{
			return std::floor(v);
		}
	}

	P_API constexpr i32 FloorToI32(float f)
	{
		return i32(Floor(f));
	}

	P_API constexpr i64 FloorToI64(double f)
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
			else if (Limits<T>::Epsilon() > Abs(v))
			{
				return v;
			}
			const i64 i = static_cast<i64>(v);
			return T(v > i ? i + 1 : i);
		}
		else
		{
			return std::ceil(v);
		}
	}
	P_API constexpr i32 CeilToI32(float f)
	{
		return i32(Ceil(f));
	}
	P_API constexpr i64 CeilToI64(double f)
	{
		return i64(Ceil(f));
	}


	template<FloatingPoint T>
	static constexpr T Round(T v)
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
			return (v >= 0.0) ? i64(v + 0.5) : i64(v - 0.5);
		}
		else
		{
			return std::round(v);
		}
	}
	P_API constexpr i32 RoundToI32(float v)
	{
		return i32(Round(v));
	}
	P_API constexpr i64 RoundToI64(double v)
	{
		return i64(Round(v));
	}

	/**
	 * Converts a floating point number to an integer which is further from zero, "larger" in
	 * absolute value: 0.1 becomes 1, -0.1 becomes -1
	 * @param f Floating point value to convert
	 * @return The rounded integer
	 */
	inline P_API float RoundFromZero(float f)
	{
		return (f < 0.0f) ? Floor(f) : Ceil(f);
	}

	inline P_API double RoundFromZero(double d)
	{
		return (d < 0.0) ? Floor(d) : Ceil(d);
	}

	/**
	 * Converts a floating point number to an integer which is closer to zero, "smaller" in
	 * absolute value: 0.1 becomes 0, -0.1 becomes 0
	 * @param v Floating point value to convert
	 * @return The rounded integer
	 */
	inline P_API float RoundToZero(float f)
	{
		return (f < 0.0f) ? Ceil(f) : Floor(f);
	}

	inline P_API double RoundToZero(double d)
	{
		return (d < 0.0) ? Ceil(d) : Floor(d);
	}

	/**
	 * Converts a floating point number to an integer which is more negative: 0.1 becomes 0,
	 * -0.1 becomes -1
	 * @param F		Floating point value to convert
	 * @return		The rounded integer
	 */
	inline P_API float RoundToNegativeInfinity(float f)
	{
		return Floor(f);
	}

	inline P_API double RoundToNegativeInfinity(double d)
	{
		return Floor(d);
	}

	/**
	 * Converts a floating point number to an integer which is more positive: 0.1 becomes 1,
	 * -0.1 becomes 0
	 * @param f		Floating point value to convert
	 * @return		The rounded integer
	 */
	inline P_API float RoundToPositiveInfinity(float f)
	{
		return Ceil(f);
	}

	inline P_API double RoundToPositiveInfinity(double d)
	{
		return Ceil(d);
	}

	/**
	 * Returns the fractional part of a float.
	 * @param f	Floating point value to convert
	 * @return		A float between >=0 and < 1.
	 */
	inline P_API float Frac(float f)
	{
		return f - p::Floor(f);
	}
	inline P_API double Frac(double d)
	{
		return d - p::Floor(d);
	}


	template<typename T>
	static T Sqrt(T val)
	{
		return std::sqrt(val);
	}

	inline P_API float InvSqrt(float x)
	{
		return 1.f / Sqrt(x);
	}
	inline P_API double InvSqrt(double x)
	{
		return 1. / Sqrt(x);
	}
	inline P_API float InvSqrt(i32 x)
	{
		return 1.f / Sqrt(x);
	}
	inline P_API double InvSqrt(i64 x)
	{
		return 1. / Sqrt(x);
	}

	template<typename T>
	static constexpr T Square(T val)
	{
		return val * val;
	}

	template<SignedIntegral T>
	P_API constexpr T Pow(T value, u32 power)
	{
		if (power == 0)
		{
			return value >= 0 ? 1 : -1;
		}

		T result = value;
		for (u32 i = 1; i < power; ++i)
		{
			result *= value;
		}
		return result;
	}
	template<UnsignedIntegral T>
	P_API constexpr T Pow(T value, u32 power)
	{
		if (power == 0)
		{
			return 1u;
		}

		T result = value;
		for (u32 i = 1; i < power; ++i)
		{
			result *= value;
		}
		return result;
	}

	template<FloatingPoint V, Number P>
	static constexpr V Pow(V value, P power)
	{
		if (std::is_constant_evaluated() && Integral<P>)
		{
			if (power == 0)
			{
				return V(1);
			}

			V result = value;
			for (P i = 1; i < power; ++i)
			{
				result *= value;
			}
			return result;
		}
		return std::pow(value, power);
	}


	// Same as Max but with N arguments
	template<typename Type, typename... Args>
	static constexpr Type Max(Type a, Type b, Args... args)
	{
		return Max(a, Max(b, Fwd<Args>(args)...));
	}

	// Same as Min but with N arguments
	template<typename Type, typename... Args>
	static constexpr Type Min(Type a, Type b, Args... args)
	{
		return Min(a, Min(b, Fwd<Args>(args)...));
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
	static inline constexpr float Mod(Type a, Type b)
	{
		if constexpr (IsSame<Type, float>)
		{
			return fmodf(a, b);
		}
		else if constexpr (IsSame<Type, double>)
		{
			return fmod(a, b);
		}
		else
		{
			return a - b * Floor(a / b);
		}
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

	P_API void SinCos(float value, float& outSin, float& outCos);
	P_API float Sin(float value);
	P_API float Cos(float value);

	P_API float Atan2(float Y, float X);

	/**
	 * Computes the ASin of a scalar value.
	 *
	 * @param Value  input angle
	 * @return ASin of Value
	 */
	inline P_API float FastAsin(float Value)
	{
		// Note:  We use FASTASIN_HALF_PI instead of HALF_PI inside of FastASin(), since it was
		// the value that accompanied the minimax coefficients below. It is important to use
		// exactly the same value in all places inside this function to ensure that
		// FastASin(0.0f) == 0.0f. For comparison:
		//		Half PI				== 1.57079632679f == 0x3fC90FDB
		//		FastAsin Half PI	== 1.5707963050f  == 0x3fC90FDA
		static constexpr float fastAsinHalfPi = 1.5707963050f;

		// Clamp input to [-1,1].
		bool nonnegative = (Value >= 0.0f);
		float x          = Abs(Value);
		float omx        = 1.0f - x;
		if (omx < 0.0f)
		{
			omx = 0.0f;
		}
		float root = Sqrt(omx);
		// 7-degree minimax approximation
		// clang-format off
			float result = ((((((-0.0012624911f * x + 0.0066700901f)
				* x - 0.0170881256f) * x + 0.0308918810f)
				* x - 0.0501743046f) * x + 0.0889789874f)
				* x - 0.2145988016f) * x + fastAsinHalfPi;
		// clang-format on

		result *= root;    // acos(|x|)
		// acos(x) = pi - acos(-x) when x < 0, asin(x) = pi/2 - acos(x)
		return (nonnegative ? fastAsinHalfPi - result : result - fastAsinHalfPi);
	}

	inline P_API bool NearlyEqual(float a, float b, float tolerance = smallNumber)
	{
		return Abs(b - a) <= tolerance;
	}

	inline P_API float Log(float k)
	{
		return std::log(k);
	}
	inline P_API float Log(float k, float base)
	{
		return Log(k) / Log(base);
	}
	inline P_API double Log(double k)
	{
		return std::log(k);
	}
	inline P_API double Log(double k, double base)
	{
		return Log(k) / Log(base);
	}
	template<Integral T>
	static double Log(T k)
	{
		return std::log(k);
	}

	inline P_API float Log2(float k)
	{
		return std::log2f(k);
	}
	inline P_API double Log2(double k)
	{
		return std::log2(k);
	}
	template<Integral T>
	static double Log2(T k)
	{
		return std::log2(double(k));
	}

	inline P_API const float Exp2(const float k)
	{
		return std::exp2(k);
	}
	inline P_API const double Exp2(const double k)
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

	P_API float ClampAngle(float a);

	P_API float NormalizeAngle(float a);

	P_API float ClampAngle(float a, float min, float max);

	P_API constexpr i32 CountBits(u64 value)
	{
#if P_PLATFORM_LINUX || P_PLATFORM_MACOS
		return __builtin_popcountll(value);
#else
		// https://en.wikipedia.org/wiki/Hamming_weight
		value -= (value >> 1) & 0x5555555555555555ull;
		value = (value & 0x3333333333333333ull) + ((value >> 2) & 0x3333333333333333ull);
		value = (value + (value >> 4)) & 0x0f0f0f0f0f0f0f0full;
		return (value * 0x0101010101010101) >> 56;
#endif
	}


#pragma region Vectors
	template<u32 size, Number T>
	struct Vec
	{
		Vec() = delete;
	};


	template<Number T>
	struct Vec<2, T>
	{
		using Type = T;

	public:
		T x;
		T y;


	public:
		constexpr Vec() : x{0}, y{0} {}
		constexpr Vec(p::Undefined) {}
		constexpr Vec(T x, T y) : x{x}, y{y} {}
		template<typename T2>
		explicit constexpr Vec(const Vec<2, T2>& other) requires(std::is_convertible_v<T2, T>)
		    : x{T(other.x)}, y{T(other.y)}
		{}

		constexpr Vec<3, T> XY() const
		{
			return {x, y, T(0)};
		}

		constexpr Vec<3, T> XZ() const
		{
			return {x, T(0), y};
		}

		constexpr u32 Num() const
		{
			return 2;
		}

		constexpr T Length() const
		{
			return Sqrt(LengthSquared());
		}

		constexpr T LengthSquared() const
		{
			return x * x + y * y;
		}

		static T Distance(const Vec& one, const Vec& other)
		{
			return Sqrt(DistanceSqrt(one, other));
		}
		static T DistanceSqrt(const Vec& one, const Vec& other)
		{
			return Square(other.x - one.x) + Square(other.y - one.y);
		}

		bool Equals(const Vec& other, float tolerance = smallNumber) const
		{
			return NearlyEqual(x, other.x, tolerance) && NearlyEqual(y, other.y, tolerance);
		}

		Vec& Normalize()
		{
			const T lengthSquared = LengthSquared();
			if (lengthSquared > 0.f)
			{
				const auto scale = InvSqrt(lengthSquared);
				x *= scale;
				y *= scale;
			}
			return *this;
		}

		static T Cross(const Vec& a, const Vec& b)
		    requires(FloatingPoint<T>)    // 'Cross' accepts only floating-point inputs
		{
			return a.x * b.y - b.x * a.y;
		}

		static T Dot(const Vec& a, const Vec& b)
		    requires(FloatingPoint<T>)    // 'Dot' accepts only floating-point inputs
		{
			const Vec tmp(a * b);
			return tmp.x + tmp.y;
		}

		constexpr Vec Floor() const
		{
			return {p::Floor(x), p::Floor(y)};
		}

		static Vec Mid(Vec one, Vec other)
		{
			return one + ((other - one) * 0.5f);
		}

		constexpr Vec Reflect(Vec normal) const
		{
			return *this - (*this * normal) * 2.f * normal;
		}

		static constexpr Vec FromAngle(float angle)
		{
			return FromAngleRad(angle * degToRad);
		}
		constexpr Vec Rotate(float angle) const
		{
			return RotateRad(angle * degToRad);
		}

		static constexpr Vec FromAngleRad(float angle)
		{
			return {std::cos(angle), std::sin(angle)};
		}

		constexpr Vec RotateRad(float angle) const
		{
			const float aCos = std::cos(angle);
			const float aSin = std::sin(angle);
			return {T(x * aCos - y * aSin), T(x * aSin + y * aCos)};
		}

		constexpr float AngleRad() const
		{
			return Atan2(y, x);
		}
		constexpr float Angle() const
		{
			return AngleRad() * radToDeg;
		}

		T* Data()
		{
			return &x;
		}
		const T* Data() const
		{
			return &x;
		}

		static constexpr Vec Zero()
		{
			return {};
		}
		static constexpr Vec One()
		{
			return {T(1), T(1)};
		}


		template<Number T2>
		constexpr Vec operator+(const Vec<2, T2>& other) const
		{
			return {x + other.x, y + other.y};
		}
		template<Number T2>
		constexpr Vec operator-(const Vec<2, T2>& other) const
		{
			return {x - other.x, y - other.y};
		}
		template<Number T2>
		constexpr Vec operator*(const Vec<2, T2>& other) const
		{
			return {x * other.x, y * other.y};
		}
		template<Number T2>
		constexpr Vec operator/(const Vec<2, T2>& other) const
		{
			return {x / other.x, y / other.y};
		}

		template<Number T2>
		constexpr Vec& operator+=(const Vec<2, T2>& other)
		{
			x += other.x;
			y += other.y;
			return *this;
		}
		template<Number T2>
		constexpr Vec& operator-=(const Vec<2, T2>& other)
		{
			x -= other.x;
			y -= other.y;
			return *this;
		}
		template<Number T2>
		constexpr Vec& operator*=(const Vec<2, T2>& other)
		{
			x *= other.x;
			y *= other.y;
			return *this;
		}
		template<Number T2>
		constexpr Vec& operator/=(const Vec<2, T2>& other)
		{
			x /= other.x;
			y /= other.y;
			return *this;
		}

		template<Number T2>
		constexpr Vec operator+(T2 other) const
		{
			return {x + other, y + other};
		}
		template<Number T2>
		constexpr Vec operator-(T2 other) const
		{
			return {x - other, y - other};
		}
		template<Number T2>
		constexpr Vec operator*(T2 other) const
		{
			return {x * other, y * other};
		}
		template<Number T2>
		constexpr Vec operator/(T2 other) const
		{
			return {x / other, y / other};
		}

		template<Number T2>
		constexpr Vec& operator+=(T2 other)
		{
			x += other;
			y += other;
			return *this;
		}
		template<Number T2>
		constexpr Vec& operator-=(T2 other)
		{
			x -= other;
			y -= other;
			return *this;
		}
		template<Number T2>
		constexpr Vec& operator*=(T2 other)
		{
			x *= other;
			y *= other;
			return *this;
		}
		template<Number T2>
		constexpr Vec& operator/=(T2 other)
		{
			x /= other;
			y /= other;
			return *this;
		}

		constexpr Vec operator-() const
		{
			return {-x, -y};
		}

		bool operator==(const Vec& other) const
		{
			return x == other.x && y == other.y;
		}

		bool operator!=(const Vec& other) const
		{
			return x != other.x || y != other.y;
		}

		constexpr T& operator[](u32 i)
		{
			return (&this->x)[i];
		}
		constexpr const T& operator[](u32 i) const
		{
			return (&this->x)[i];
		}
	};

	template<Number T2>
	constexpr Vec<2, T2> operator/(T2 value, Vec<2, T2> vector)
	{
		return {value / vector.x, value / vector.y};
	}


	template<Number T>
	struct Vec<3, T>
	{
		using Type = T;

	public:
		T x;
		T y;
		T z;


	public:
		constexpr Vec() : x{0}, y{0}, z{0} {}
		constexpr Vec(p::Undefined) {}
		constexpr Vec(T x, T y, T z) : x{x}, y{y}, z{z} {}
		template<typename T2>
		explicit constexpr Vec(const Vec<3, T2>& other) requires(std::is_convertible_v<T2, T>)
		    : x{other.x}, y{other.y}, z{other.z}
		{}

		constexpr Vec<2, T> XY() const
		{
			return {x, y};
		}
		constexpr Vec<2, T> XZ() const
		{
			return {x, z};
		}

		constexpr u32 Num() const
		{
			return 3;
		}

		constexpr T Length() const
		{
			return Sqrt(LengthSquared());
		}

		constexpr T LengthSquared() const
		{
			return x * x + y * y + z * z;
		}

		static T Distance(const Vec& one, const Vec& other)
		{
			return Sqrt(DistanceSqrt(one, other));
		}
		static T DistanceSqrt(const Vec& one, const Vec& other)
		{
			return Square(other.x - one.x) + Square(other.y - one.y) + Square(other.z - one.z);
		}

		bool Equals(const Vec& other, float tolerance = smallNumber) const
		{
			return NearlyEqual(x, other.x, tolerance) && NearlyEqual(y, other.y, tolerance)
			    && NearlyEqual(z, other.z, tolerance);
		}

		Vec& Normalize()
		{
			const T lengthSqrt = LengthSquared();
			if (lengthSqrt > 0.f)
			{
				const auto scale = InvSqrt(lengthSqrt);
				x *= scale;
				y *= scale;
				z *= scale;
			}
			return *this;
		}

		static Vec Cross(const Vec& a, const Vec& b)
		    requires(FloatingPoint<T>)    // 'Cross' accepts only floating-point inputs
		{
			return {a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x, a.x * b.y - b.x * a.y};
		}

		static T Dot(const Vec& a, const Vec& b)
		    requires(FloatingPoint<T>)    // 'Dot' accepts only floating-point inputs
		{
			const Vec tmp(a * b);
			return tmp.x + tmp.y + tmp.z;
		}

		constexpr Vec Floor() const
		{
			return {p::Floor(x), p::Floor(y), p::Floor(z)};
		}

		static Vec Mid(Vec one, Vec other)
		{
			return one + ((other - one) * 0.5f);
		}

		T* Data()
		{
			return &x;
		}
		const T* Data() const
		{
			return &x;
		}

		static constexpr Vec Zero()
		{
			return {};
		};
		static constexpr Vec One()
		{
			return {T(1), T(1), T(1)};
		}
		static constexpr Vec Forward()
		{
			return {T(0), T(1), T(0)};
		}
		static constexpr Vec Right()
		{
			return {T(1), T(0), T(0)};
		}
		static constexpr Vec Up()
		{
			return {T(0), T(0), T(1)};
		}

		template<Number T2>
		constexpr Vec operator+(const Vec<3, T2>& other) const
		{
			return {x + other.x, y + other.y, z + other.z};
		}
		template<Number T2>
		constexpr Vec operator-(const Vec<3, T2>& other) const
		{
			return {x - other.x, y - other.y, z - other.z};
		}
		template<Number T2>
		constexpr Vec operator*(const Vec<3, T2>& other) const
		{
			return {x * other.x, y * other.y, z * other.z};
		}
		template<Number T2>
		constexpr Vec operator/(const Vec<3, T2>& other) const
		{
			return {x / other.x, y / other.y, z / other.z};
		}

		template<Number T2>
		constexpr Vec& operator+=(const Vec<3, T2>& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}
		template<Number T2>
		constexpr Vec& operator-=(const Vec<3, T2>& other)
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}
		template<Number T2>
		constexpr Vec& operator*=(const Vec<3, T2>& other)
		{
			x *= other.x;
			y *= other.y;
			z *= other.z;
			return *this;
		}
		template<Number T2>
		constexpr Vec& operator/=(const Vec<3, T2>& other)
		{
			x /= other.x;
			y /= other.y;
			z /= other.z;
			return *this;
		}

		template<Number T2>
		constexpr Vec operator+(T2 other) const
		{
			return {x + other, y + other, z + other};
		}
		template<Number T2>
		constexpr Vec operator-(T2 other) const
		{
			return {x - other, y - other, z - other};
		}
		template<Number T2>
		constexpr Vec operator*(T2 other) const
		{
			return {x * other, y * other, z * other};
		}
		template<Number T2>
		constexpr Vec operator/(T2 other) const
		{
			return {x / other, y / other, z / other};
		}

		template<Number T2>
		constexpr Vec& operator+=(T2 other)
		{
			x += other;
			y += other;
			z += other;
			return *this;
		}
		template<Number T2>
		constexpr Vec& operator-=(T2 other)
		{
			x -= other;
			y -= other;
			z -= other;
			return *this;
		}
		template<Number T2>
		constexpr Vec& operator*=(T2 other)
		{
			x *= other;
			y *= other;
			z *= other;
			return *this;
		}
		template<Number T2>
		constexpr Vec& operator/=(T2 other)
		{
			x /= other;
			y /= other;
			z /= other;
			return *this;
		}

		constexpr Vec operator-() const
		{
			return {-x, -y, -z};
		}

		bool operator==(const Vec& other) const
		{
			return x == other.x && y == other.y && z == other.z;
		}

		bool operator!=(const Vec& other) const
		{
			return x != other.x || y != other.y || z != other.z;
		}

		constexpr T& operator[](u32 i)
		{
			return (&this->x)[i];
		}
		constexpr const T& operator[](u32 i) const
		{
			return (&this->x)[i];
		}
	};

	template<Number T2>
	constexpr Vec<3, T2> operator/(T2 value, Vec<3, T2> vector)
	{
		return {value / vector.x, value / vector.y, value / vector.z};
	}


	template<Number T>
	struct Vec<4, T>
	{
		using Type = T;

	public:
		T x;
		T y;
		T z;
		T w;


	public:
		constexpr Vec() : x{0}, y{0}, z{0}, w{0} {}
		constexpr Vec(p::Undefined) {}
		constexpr Vec(T x, T y, T z, T w) : x{x}, y{y}, z{z}, w{w} {}
		template<typename T2>
		explicit constexpr Vec(const Vec<4, T2>& other) requires(std::is_convertible_v<T2, T>)
		    : x{other.x}, y{other.y}, z{other.z}, w{other.w}
		{}

		constexpr Vec<3, T> XYZ() const
		{
			return {x, y, z};
		}

		bool Equals(const Vec& other, float tolerance = smallNumber) const
		{
			return NearlyEqual(x, other.x, tolerance) && NearlyEqual(y, other.y, tolerance)
			    && NearlyEqual(z, other.z, tolerance) && NearlyEqual(w, other.w, tolerance);
		}

		static T Dot(const Vec& a, const Vec& b)
		    requires(FloatingPoint<T>)    // 'Dot' accepts only floating-point inputs
		{
			const Vec tmp(a * b);
			return (tmp.x + tmp.y) + (tmp.z + tmp.w);
		}

		constexpr Vec Floor() const
		{
			return {Floor(x), Floor(y), Floor(z), Floor(w)};
		}

		static Vec Mid(Vec one, Vec other)
		{
			return one + ((other - one) * 0.5f);
		}

		T* Data()
		{
			return &x;
		}
		const T* Data() const
		{
			return &x;
		}

		static constexpr Vec Zero()
		{
			return {};
		};
		static constexpr Vec One()
		{
			return {T(1), T(1), T(1), T(1)};
		}

		constexpr T& operator[](u32 i)
		{
			return (&this->x)[i];
		}
		constexpr const T& operator[](u32 i) const
		{
			return (&this->x)[i];
		}
	};

	template<Number T2>
	constexpr Vec<4, T2> operator/(T2 value, Vec<4, T2> vector)
	{
		return {value / vector.x, value / vector.y, value / vector.z, value / vector.w};
	}


	using v2  = Vec<2, float>;
	using v3  = Vec<3, float>;
	using v4  = Vec<4, float>;
	using v2d = Vec<2, double>;
	using v3d = Vec<3, double>;
	using v4d = Vec<4, double>;

	using v2_i8 = Vec<2, i8>;
	using v2_u8 = Vec<2, u8>;
	using v3_i8 = Vec<3, i8>;
	using v3_u8 = Vec<3, u8>;
	using v4_i8 = Vec<4, i8>;
	using v4_u8 = Vec<4, u8>;

	using v2_i32 = Vec<2, i32>;
	using v2_u32 = Vec<2, u32>;
	using v3_i32 = Vec<3, i32>;
	using v3_u32 = Vec<3, u32>;
	using v4_i32 = Vec<4, i32>;
	using v4_u32 = Vec<4, u32>;

	using v2_i64 = Vec<2, i64>;
	using v2_u64 = Vec<2, u64>;
	using v3_i64 = Vec<3, i64>;
	using v3_u64 = Vec<3, u64>;
	using v4_i64 = Vec<4, i64>;
	using v4_u64 = Vec<4, u64>;


	template<u32 size, Number T>
	struct TAABB
	{
		static_assert(size >= 2 && size <= 4, "AABB size must be between 2 and 4");
		Vec<size, T> min;
		Vec<size, T> max;


		TAABB() = default;
		constexpr TAABB(p::Undefined) : min{p::Undefined{}}, max{p::Undefined{}} {}
		constexpr TAABB(Vec<size, T> min, Vec<size, T> max) : min{min}, max{max} {}


		// Limit this AABB by another AABB
		void Cut(const TAABB& other)
		{
			for (u32 i = 0; i < size; ++i)
			{
				min[i] = Clamp(min[i], other.min[i], other.max[i]);
			}
			for (u32 i = 0; i < size; ++i)
			{
				max[i] = Clamp(max[i], other.min[i], other.max[i]);
			}
		}

		void Merge(const Vec<size, T>& point)
		{
			for (u32 i = 0; i < size; ++i)
			{
				if (point[i] < min[i])
				{
					min[i] = point[i];
				}
			}
			for (u32 i = 0; i < size; ++i)
			{
				if (point[i] > max[i])
				{
					max[i] = point[i];
				}
			}
		}
		void Merge(const TAABB& other)
		{
			for (u32 i = 0; i < size; ++i)
			{
				if (other.min[i] < min[i])
				{
					min[i] = other.min[i];
				}
			}
			for (u32 i = 0; i < size; ++i)
			{
				if (other.max[i] > max[i])
				{
					max[i] = other.max[i];
				}
			}
		}

		// Makes this rect shrink to occupy the overlapping area with another rect
		// if this and other dont overlap, result will be an invalid rect
		void Diff(const TAABB& other)
		{
			for (u32 i = 0; i < size; ++i)
			{
				if (other.min[i] > min[i])
				{
					min[i] = other.min[i];
				}
			}
			for (u32 i = 0; i < size; ++i)
			{
				if (other.max[i] < max[i])
				{
					max[i] = other.max[i];
				}
			}
		}

		void Expand(T amount)
		{
			for (u32 i = 0; i < size; ++i)
			{
				min[i] -= amount;
			}
			for (u32 i = 0; i < size; ++i)
			{
				max[i] += amount;
			}
		}
		void Expand(const Vec<size, T>& amount)
		{
			for (u32 i = 0; i < size; ++i)
			{
				min[i] -= amount[i];
			}
			for (u32 i = 0; i < size; ++i)
			{
				max[i] += amount[i];
			}
		}

		constexpr bool Contains(const Vec<size, T>& p) const
		{
			if constexpr (size == 2)
			{
				return (p.x >= min.x && p.y >= min.y) && (p.x < max.x && p.y < max.y);
			}
			else if constexpr (size == 3)
			{
				return (p.x >= min.x && p.y >= min.y && p.z >= min.z)
				    && (p.x < max.x && p.y < max.y && p.z < max.z);
			}
			else if constexpr (size == 4)
			{
				return (p.x >= min.x && p.y >= min.y && p.z >= min.z && p.w >= min.w)
				    && (p.x < max.x && p.y < max.y && p.z < max.z && p.w < max.w);
			}
		}
		constexpr bool Contains(const TAABB& r) const requires(size == 2)
		{
			if constexpr (size == 2)
			{
				return (r.min.x >= min.x && r.min.y >= min.y)
				    && (r.max.x <= max.x && r.max.y <= max.y);
			}
			else if constexpr (size == 3)
			{
				return (r.min.x >= min.x && r.min.y >= min.y && r.min.z >= min.z)
				    && (r.max.x <= max.x && r.max.y <= max.y && r.max.z <= max.z);
			}
			else if constexpr (size == 4)
			{
				return (r.min.x >= min.x && r.min.y >= min.y && r.min.z >= min.z
				           && r.min.w >= min.w)
				    && (r.max.x <= max.x && r.max.y <= max.y && r.max.z <= max.z
				        && r.max.w <= max.w);
			}
		}
		constexpr bool Overlaps(const Vec<size, T>& p) const
		{
			if constexpr (size == 2)
			{
				return (p.x > min.x && p.y > min.y) && (p.x < max.x && p.y < max.y);
			}
			else if constexpr (size == 3)
			{
				return (p.x > min.x && p.y > min.y && p.z > min.z)
				    && (p.x < max.x && p.y < max.y && p.z < max.z);
			}
			else if constexpr (size == 4)
			{
				return (p.x > min.x && p.y > min.y && p.z > min.z && p.w > min.w)
				    && (p.x < max.x && p.y < max.y && p.z < max.z && p.w < max.w);
			}
		}
		constexpr bool Overlaps(const TAABB& r) const
		{
			if constexpr (size == 2)
			{
				return (r.min.x < max.x && r.min.y < max.y) && (r.max.x > min.x && r.max.y > min.y);
			}
			else if constexpr (size == 3)
			{
				return (r.min.x < max.x && r.min.y < max.y && r.min.z < max.z)
				    && (r.max.x > min.x && r.max.y > min.y && r.max.z > min.z);
			}
			else if constexpr (size == 4)
			{
				return (r.min.x < max.x && r.min.y < max.y && r.min.z < max.z && r.min.w < max.w)
				    && (r.max.x > min.x && r.max.y > min.y && r.max.z > min.z && r.max.w > min.w);
			}
		}

		constexpr Vec<size, T> GetSize() const
		{
			return max - min;
		}

		constexpr Vec<size, T> GetCenter() const
		{
			return min + (max - min) * 0.5f;
		}

		Vec<size, T> Clamp(const Vec<size, T>& point)
		{
			Vec<size, T> clamped;
			clamped.x = p::Clamp(point.x, min.x, max.x);
			clamped.y = p::Clamp(point.y, min.y, max.y);
			if constexpr (size >= 3)
			{
				clamped.z = p::Clamp(point.z, min.z, max.z);
			}
			return clamped;
		}


		constexpr v4 ToV4() const requires(size == 2)
		{
			return {min.x, min.y, max.x, max.y};
		}

		static constexpr TAABB Zero()
		{
			return {};
		}

		constexpr bool IsInverted() const
		{
			if constexpr (size == 2)
			{
				return min.x > max.x || min.y > max.y;
			}
			else if constexpr (size == 3)
			{
				return min.x > max.x || min.y > max.y || min.z > max.z;
			}
			else if constexpr (size == 4)
			{
				return min.x > max.x || min.y > max.y || min.z > max.z || min.w > max.w;
			}
		}
	};

	template<typename Type>
	using TRect = TAABB<2, Type>;

	template<typename Type>
	using TBox = TAABB<3, Type>;

	using Rect  = TRect<float>;
	using Box   = TBox<float>;
	using Recti = TRect<i32>;
	using Boxi  = TBox<i32>;
	using Rectu = TRect<u32>;
	using Boxu  = TBox<u32>;


	namespace Vectors
	{
		// mathematically if you have 0 scale, it should be infinite,
		// however, in practice if you have 0 scale, and relative transform doesn't make much
		// sense anymore because you should be instead of showing gigantic infinite mesh also
		// returning BIG_NUMBER causes sequential NaN issues by multiplying so we hardcode as 0
		P_API v3 GetSafeScaleReciprocal(const v3& scale, float tolerance = smallNumber);
		P_API v2 ClosestPointInLine(v2 a, v2 b, v2 point);
		P_API v3 ClosestPointInLine(v3 a, v3 b, v3 point);
	}    // namespace Vectors

	P_API v2 EvaluateCubicBezier(v2 p0, v2 p1, v2 p2, v2 p3, float t);


	struct P_API Rotator : public v3
	{
		using v3::v3;

		Rotator() {}
		Rotator(p::Undefined) : v3{p::Undefined{}} {}
		Rotator(const v3& vector) : v3{vector} {}

		float Pitch() const
		{
			return y;
		}
		float Yaw() const
		{
			return z;
		}
		float Roll() const
		{
			return x;
		}

		float& Pitch()
		{
			return y;
		}
		float& Yaw()
		{
			return z;
		}
		float& Roll()
		{
			return x;
		}

		/**
		 * Clamps an angle to the range of [0, 360).
		 *
		 * @param Angle The angle to clamp.
		 * @return The clamped angle.
		 */
		static float ClampAxis(float Angle);

		/**
		 * Clamps an angle to the range of (-180, 180].
		 *
		 * @param Angle The Angle to clamp.
		 * @return The clamped angle.
		 */
		static float NormalizeAxis(float Angle);
	};


	struct P_API Quat
	{
	public:
		float x;
		float y;
		float z;
		float w;

	public:
		// Default constructor (uninitialized)
		constexpr Quat() : x{0.f}, y{0.f}, z{0.f}, w{1.f} {}
		constexpr Quat(p::Undefined) {}
		constexpr Quat(float x, float y, float z, float w) : x{x}, y{y}, z{z}, w{w} {}

		v3 Rotate(const v3& vector) const;
		v3 Unrotate(const v3& vector) const;

		Rotator ToRotator() const;
		Rotator ToRotatorRad() const;

		v3 GetForward() const;
		v3 GetRight() const;
		v3 GetUp() const;

		Quat Inverse() const;

		float* Data()
		{
			return &x;
		}
		const float* Data() const
		{
			return &x;
		}

		bool Equals(const Quat& other, float tolerance = smallNumber) const;

		void Normalize(float tolerance = smallNumber);

		static Quat FromRotator(Rotator rotator);

		static Quat FromRotatorRad(Rotator rotator);

		static Quat Between(const v3& a, const v3& b);
		static Quat BetweenNormals(const v3& a, const v3& b);
		static Quat LookAt(const v3& origin, const v3& dest);

		static constexpr Quat Identity()
		{
			return {1, 0, 0, 0};
		}

		template<typename T>
		v3 operator*(const v3& v)
		{
			return Rotate(v);
		}
	};

	template<u32 size, Number T>
	inline p::sizet GetHash(Vec<size, T> v)
	{
		return HashBytes(&v, sizeof(Vec<size, T>));
	}
#pragma endregion Vectors
}    // namespace p
