// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Math.h"
#include "Reflection/Builders/NativeTypeBuilder.h"
#include "Serialization/ContextsFwd.h"
#include "Strings/Name.h"
#include "TypeTraits.h"


namespace Pipe
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
			return Math::Sqrt(LengthSquared());
		}

		constexpr T LengthSquared() const
		{
			return x * x + y * y;
		}

		static T Distance(const Vec& one, const Vec& other)
		{
			return Math::Sqrt(DistanceSqrt(one, other));
		}
		static T DistanceSqrt(const Vec& one, const Vec& other)
		{
			return Math::Square(other.x - one.x) + Math::Square(other.y - one.y);
		}

		bool Equals(const Vec& other, float tolerance = Math::SMALL_NUMBER) const
		{
			return Math::NearlyEqual(x, other.x, tolerance)
			    && Math::NearlyEqual(y, other.y, tolerance);
		}

		Vec& Normalize()
		{
			const T length = Length();
			if (length > 0.f)
			{
				const float scale = Math::InvSqrt(length);
				x *= scale;
				y *= scale;
			}
			return *this;
		}

		static T Cross(const Vec& a, const Vec& b) requires(
		    FloatingPoint<T>)    // 'Cross' accepts only floating-point inputs
		{
			return a.x * b.y - b.x * a.y;
		}

		static T Dot(const Vec& a, const Vec& b) requires(
		    FloatingPoint<T>)    // 'Dot' accepts only floating-point inputs
		{
			const Vec tmp(a * b);
			return tmp.x + tmp.y;
		}

		constexpr Vec Floor() const
		{
			return {Math::Floor(x), Math::Floor(y)};
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
			return Math::Sqrt(LengthSquared());
		}

		constexpr T LengthSquared() const
		{
			return x * x + y * y + z * z;
		}

		static T Distance(const Vec& one, const Vec& other)
		{
			return Math::Sqrt(DistanceSqrt(one, other));
		}
		static T DistanceSqrt(const Vec& one, const Vec& other)
		{
			return Math::Square(other.x - one.x) + Math::Square(other.y - one.y)
			     + Math::Square(other.z - one.z);
		}

		bool Equals(const Vec& other, float tolerance = Math::SMALL_NUMBER) const
		{
			return Math::NearlyEqual(x, other.x, tolerance)
			    && Math::NearlyEqual(y, other.y, tolerance)
			    && Math::NearlyEqual(z, other.z, tolerance);
		}

		Vec& Normalize()
		{
			const T lengthSqrt = LengthSquared();
			if (lengthSqrt > 0.f)
			{
				const float scale = Math::InvSqrt(lengthSqrt);
				x *= scale;
				y *= scale;
				z *= scale;
			}
			return *this;
		}

		static Vec Cross(const Vec& a, const Vec& b) requires(
		    FloatingPoint<T>)    // 'Cross' accepts only floating-point inputs
		{
			return {a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x, a.x * b.y - b.x * a.y};
		}

		static T Dot(const Vec& a, const Vec& b) requires(
		    FloatingPoint<T>)    // 'Dot' accepts only floating-point inputs
		{
			const Vec tmp(a * b);
			return tmp.x + tmp.y + tmp.z;
		}

		constexpr Vec Floor() const
		{
			return {Math::Floor(x), Math::Floor(y), Math::Floor(z)};
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
		constexpr Vec(T x, T y, T z, T w) : x{x}, y{y}, z{z}, w{w} {}
		template<typename T2>
		explicit constexpr Vec(const Vec<4, T2>& other) requires(std::is_convertible_v<T2, T>)
		    : x{other.x}, y{other.y}, z{other.z}, w{other.w}
		{}

		constexpr Vec<3, T> XYZ() const
		{
			return {x, y, z};
		}

		bool Equals(const Vec& other, float tolerance = Math::SMALL_NUMBER) const
		{
			return Math::NearlyEqual(x, other.x, tolerance)
			    && Math::NearlyEqual(y, other.y, tolerance)
			    && Math::NearlyEqual(z, other.z, tolerance)
			    && Math::NearlyEqual(w, other.w, tolerance);
		}

		static T Dot(const Vec& a, const Vec& b) requires(
		    FloatingPoint<T>)    // 'Dot' accepts only floating-point inputs
		{
			const Vec tmp(a * b);
			return (tmp.x + tmp.y) + (tmp.z + tmp.w);
		}

		constexpr Vec Floor() const
		{
			return {Math::Floor(x), Math::Floor(y), Math::Floor(z), Math::Floor(w)};
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


	using v2 = Vec<2, float>;
	using v3 = Vec<3, float>;
	using v4 = Vec<4, float>;

	using v2_u8 = Vec<2, u8>;
	using v3_u8 = Vec<3, u8>;
	using v4_u8 = Vec<4, u8>;

	using v2_i32 = Vec<2, i32>;
	using v3_i32 = Vec<3, i32>;
	using v4_i32 = Vec<4, i32>;

	using v2_u32 = Vec<2, u32>;
	using v3_u32 = Vec<3, u32>;
	using v4_u32 = Vec<4, u32>;


	/** Reflected vectors */
	REFLECT_NATIVE_TYPE(v2);
	REFLECT_NATIVE_TYPE(v2_u32);
	REFLECT_NATIVE_TYPE(v3);


	template<u32 size, Number T>
	struct TAABB
	{
		static_assert(size >= 2 && size <= 4, "AABB size must be between 2 and 4");
		Vec<size, T> min;
		Vec<size, T> max;


		TAABB() = default;
		constexpr TAABB(Vec<size, T> min, Vec<size, T> max) : min{min}, max{max} {}


		// Limit this AABB by another AABB
		void Cut(const TAABB& other)
		{
			for (u32 i = 0; i < size; ++i)
			{
				min[i] = Math::Clamp(min[i], other.min[i], other.max[i]);
			}
			for (u32 i = 0; i < size; ++i)
			{
				max[i] = Math::Clamp(max[i], other.min[i], other.max[i]);
			}
		}

		void Add(const Vec<size, T>& point)
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
		void Add(const TAABB& other)
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


	CORE_API void Read(Serl::ReadContext& ct, v2& val);
	CORE_API void Write(Serl::WriteContext& ct, v2 val);

	CORE_API void Read(Serl::ReadContext& ct, v2_u32& val);
	CORE_API void Write(Serl::WriteContext& ct, v2_u32 val);

	CORE_API void Read(Serl::ReadContext& ct, v3& val);
	CORE_API void Write(Serl::WriteContext& ct, const v3& val);


	namespace Vectors
	{
		// mathematically if you have 0 scale, it should be infinite,
		// however, in practice if you have 0 scale, and relative transform doesn't make much
		// sense anymore because you should be instead of showing gigantic infinite mesh also
		// returning BIG_NUMBER causes sequential NaN issues by multiplying so we hardcode as 0
		CORE_API v3 GetSafeScaleReciprocal(const v3& scale, float tolerance = Math::SMALL_NUMBER);
		CORE_API v2 ClosestPointInLine(v2 a, v2 b, v2 point);
		CORE_API v3 ClosestPointInLine(v3 a, v3 b, v3 point);
	}    // namespace Vectors
}    // namespace Pipe
