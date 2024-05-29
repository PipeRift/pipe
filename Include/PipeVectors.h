// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Utility.h"
#include "PipeMath.h"
#include "PipeSerializeFwd.h"


/// @OPTIMIZE: Try to remove this include
#include "Pipe/Core/String.h"

#include <format>


namespace p
{
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
				const float scale = InvSqrt(lengthSquared);
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
				const float scale = InvSqrt(lengthSqrt);
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

	using v2_u8 = Vec<2, u8>;
	using v3_u8 = Vec<3, u8>;
	using v4_u8 = Vec<4, u8>;

	using v2_i32 = Vec<2, i32>;
	using v3_i32 = Vec<3, i32>;
	using v4_i32 = Vec<4, i32>;

	using v2_u32 = Vec<2, u32>;
	using v3_u32 = Vec<3, u32>;
	using v4_u32 = Vec<4, u32>;


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
					min[i] = point[i];
			}
			for (u32 i = 0; i < size; ++i)
			{
				if (point[i] > max[i])
					max[i] = point[i];
			}
		}
		void Merge(const TAABB& other)
		{
			for (u32 i = 0; i < size; ++i)
			{
				if (other.min[i] < min[i])
					min[i] = other.min[i];
			}
			for (u32 i = 0; i < size; ++i)
			{
				if (other.max[i] > max[i])
					max[i] = other.max[i];
			}
		}

		// Makes this rect shrink to occupy the overlapping area with another rect
		// if this and other dont overlap, result will be an invalid rect
		void Diff(const TAABB& other)
		{
			for (u32 i = 0; i < size; ++i)
			{
				if (other.min[i] > min[i])
					min[i] = other.min[i];
			}
			for (u32 i = 0; i < size; ++i)
			{
				if (other.max[i] < max[i])
					max[i] = other.max[i];
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
				return (p.x >= min.x && p.y >= min.y) && (p.x < max.x && p.y < max.y);
			else if constexpr (size == 3)
				return (p.x >= min.x && p.y >= min.y && p.z >= min.z)
				    && (p.x < max.x && p.y < max.y && p.z < max.z);
			else if constexpr (size == 4)
				return (p.x >= min.x && p.y >= min.y && p.z >= min.z && p.w >= min.w)
				    && (p.x < max.x && p.y < max.y && p.z < max.z && p.w < max.w);
		}
		constexpr bool Contains(const TAABB& r) const requires(size == 2)
		{
			if constexpr (size == 2)
				return (r.min.x >= min.x && r.min.y >= min.y)
				    && (r.max.x <= max.x && r.max.y <= max.y);
			else if constexpr (size == 3)
				return (r.min.x >= min.x && r.min.y >= min.y && r.min.z >= min.z)
				    && (r.max.x <= max.x && r.max.y <= max.y && r.max.z <= max.z);
			else if constexpr (size == 4)
				return (r.min.x >= min.x && r.min.y >= min.y && r.min.z >= min.z
				           && r.min.w >= min.w)
				    && (r.max.x <= max.x && r.max.y <= max.y && r.max.z <= max.z
				        && r.max.w <= max.w);
		}
		constexpr bool Overlaps(const Vec<size, T>& p) const
		{
			if constexpr (size == 2)
				return (p.x > min.x && p.y > min.y) && (p.x < max.x && p.y < max.y);
			else if constexpr (size == 3)
				return (p.x > min.x && p.y > min.y && p.z > min.z)
				    && (p.x < max.x && p.y < max.y && p.z < max.z);
			else if constexpr (size == 4)
				return (p.x > min.x && p.y > min.y && p.z > min.z && p.w > min.w)
				    && (p.x < max.x && p.y < max.y && p.z < max.z && p.w < max.w);
		}
		constexpr bool Overlaps(const TAABB& r) const
		{
			if constexpr (size == 2)
				return (r.min.x < max.x && r.min.y < max.y) && (r.max.x > min.x && r.max.y > min.y);
			else if constexpr (size == 3)
				return (r.min.x < max.x && r.min.y < max.y && r.min.z < max.z)
				    && (r.max.x > min.x && r.max.y > min.y && r.max.z > min.z);
			else if constexpr (size == 4)
				return (r.min.x < max.x && r.min.y < max.y && r.min.z < max.z && r.min.w < max.w)
				    && (r.max.x > min.x && r.max.y > min.y && r.max.z > min.z && r.max.w > min.w);
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
				return min.x > max.x || min.y > max.y;
			else if constexpr (size == 3)
				return min.x > max.x || min.y > max.y || min.z > max.z;
			else if constexpr (size == 4)
				return min.x > max.x || min.y > max.y || min.z > max.z || min.w > max.w;
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
		PIPE_API v3 GetSafeScaleReciprocal(const v3& scale, float tolerance = smallNumber);
		PIPE_API v2 ClosestPointInLine(v2 a, v2 b, v2 point);
		PIPE_API v3 ClosestPointInLine(v3 a, v3 b, v3 point);
	}    // namespace Vectors

	PIPE_API v2 EvaluateCubicBezier(v2 p0, v2 p1, v2 p2, v2 p3, float t);


	struct PIPE_API Rotator : public v3
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


	struct PIPE_API Quat
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

}    // namespace p


template<typename T>
struct std::formatter<p::Vec<2, T>>
{
	p::String formatStr;

	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
	{
		auto begin = ctx.begin();
		if (begin == ctx.end())
		{
			formatStr = "({}, {})";
			return begin;
		}

		auto end = begin;
		while (*begin != '{')
		{
			--begin;
		}
		while (*end != '}')
		{
			++end;
		}
		const p::StringView valueFormat{begin, end + 1};
		formatStr = std::format("({}, {})", valueFormat, valueFormat);
		return end;
	}

	template<typename FormatContext>
	auto format(const p::Vec<2, T>& v, FormatContext& ctx) const
	{
		return std::vformat_to(ctx.out(), formatStr, std::make_format_args(v.x, v.y));
	}
};

template<typename T>
struct std::formatter<p::Vec<3, T>>
{
	p::String formatStr;

	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
	{
		auto begin = ctx.begin();
		if (begin == ctx.end())
		{
			formatStr = "({}, {}, {})";
			return begin;
		}

		auto end = begin;
		while (*begin != '{')
		{
			--begin;
		}
		while (*end != '}')
		{
			++end;
		}
		const p::StringView valueFormat{begin, end + 1};
		formatStr = std::format("({}, {}, {})", valueFormat, valueFormat, valueFormat);
		return end;
	}
	template<typename FormatContext>
	auto format(const p::Vec<3, T>& v, FormatContext& ctx) const
	{
		return vformat_to(ctx.out(), formatStr, std::make_format_args(v.x, v.y, v.z));
	}
};
