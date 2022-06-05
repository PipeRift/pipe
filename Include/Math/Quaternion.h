// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Math.h"
#include "Math/Vector.h"
#include "Reflection/Builders/NativeTypeBuilder.h"
#include "Serialization/ContextsFwd.h"


namespace p::math
{
	struct Rotator;

	struct Quat
	{
	public:
		float x;
		float y;
		float z;
		float w;

	public:
		// Default constructor (uninitialized)
		constexpr Quat() {}
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

		bool Equals(const Quat& other, float tolerance = math::SMALL_NUMBER) const;

		void Normalize(float tolerance = math::SMALL_NUMBER);

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

	inline Quat Quat::Inverse() const
	{
		return {-x, -y, -z, w};
	}

	void Read(serl::ReadContext& ct, Quat& val);
	void Write(serl::WriteContext& ct, const Quat& val);
}    // namespace p::math

namespace p
{
	using namespace p::math;
}

REFLECT_NATIVE_TYPE(p::Quat);
