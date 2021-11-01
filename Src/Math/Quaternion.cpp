// Copyright 2015-2021 Piperift - All rights reserved

#include "Math/Quaternion.h"

#include "Math/Rotator.h"
#include "Serialization/Contexts.h"


namespace Rift
{
	v3 Quat::Rotate(const v3& v) const
	{
		const v3 q(x, y, z);
		const v3 t = v3::Cross(q, v) * 2.f;
		return v + (t * w) + v3::Cross(q, t);
	}

	v3 Quat::Unrotate(const v3& v) const
	{
		const v3 q(-x, -y, -z);    // Inverse
		const v3 t = v3::Cross(q, v) * 2.f;
		return v + (t * w) + v3::Cross(q, t);
	}
	v3 Quat::GetForward() const
	{
		return Rotate(v3::Forward());
	}

	v3 Quat::GetRight() const
	{
		return Rotate(v3::Right());
	}

	v3 Quat::GetUp() const
	{
		return Rotate(v3::Up());
	}

	Rotator Quat::ToRotator() const
	{
		const float SingularityTest = z * x - w * y;
		const float YawY            = 2.f * (w * z + x * y);
		const float YawX            = (1.f - 2.f * (Math::Square(y) + Math::Square(z)));

		// reference
		// http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
		// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/

		// this value was found from experience, the above websites recommend different values
		// but that isn't the case for us, so I went through different testing, and finally found
		// the case where both of world lives happily.
		const float SINGULARITY_THRESHOLD = 0.4999995f;
		Rotator RotatorFromQuat;

		if (SingularityTest < -SINGULARITY_THRESHOLD)
		{
			RotatorFromQuat.Pitch() = -90.f;
			RotatorFromQuat.Yaw()   = Math::Atan2(YawY, YawX) * Math::RADTODEG;
			RotatorFromQuat.Roll()  = Rotator::NormalizeAxis(
			     -RotatorFromQuat.Yaw() - (2.f * Math::Atan2(x, w) * Math::RADTODEG));
		}
		else if (SingularityTest > SINGULARITY_THRESHOLD)
		{
			RotatorFromQuat.Pitch() = 90.f;
			RotatorFromQuat.Yaw()   = Math::Atan2(YawY, YawX) * Math::RADTODEG;
			RotatorFromQuat.Roll()  = Rotator::NormalizeAxis(
			     RotatorFromQuat.Yaw() - (2.f * Math::Atan2(x, w) * Math::RADTODEG));
		}
		else
		{
			RotatorFromQuat.Pitch() = Math::FastAsin(2.f * (SingularityTest)) * Math::RADTODEG;
			RotatorFromQuat.Yaw()   = Math::Atan2(YawY, YawX) * Math::RADTODEG;
			RotatorFromQuat.Roll()  = Math::Atan2(-2.f * (w * x + y * z),
			                              (1.f - 2.f * (Math::Square(x) + Math::Square(y))))
			                       * Math::RADTODEG;
		}

		return RotatorFromQuat;
	}

	Rotator Quat::ToRotatorRad() const
	{
		return ToRotator() * Math::DEGTORAD;
	}

	bool Quat::Equals(const Quat& other, float tolerance) const
	{
		return Math::NearlyEqual(x, other.x, tolerance) && Math::NearlyEqual(y, other.y, tolerance)
		    && Math::NearlyEqual(z, other.z, tolerance) && Math::NearlyEqual(w, other.w, tolerance);
	}

	void Quat::Normalize(float tolerance)
	{
		const float lengthSqrt = x * x + y * y + z * z + w * w;
		if (lengthSqrt >= tolerance)
		{
			const float scale = Math::InvSqrt(lengthSqrt);
			x *= scale;
			y *= scale;
			z *= scale;
			w *= scale;
		}
		else
		{
			*this = Identity();
		}
	}

	Quat Quat::FromRotator(Rotator rotator)
	{
		const float DIVIDE_BY_2 = Math::DEGTORAD / 2.f;
		float SP, SY, SR;
		float CP, CY, CR;

		Math::SinCos(rotator.Pitch() * DIVIDE_BY_2, SP, CP);
		Math::SinCos(rotator.Yaw() * DIVIDE_BY_2, SY, CY);
		Math::SinCos(rotator.Roll() * DIVIDE_BY_2, SR, CR);

		Quat RotationQuat{CR * CP * CY + SR * SP * SY, CR * SP * SY - SR * CP * CY,
		    -CR * SP * CY - SR * CP * SY, CR * CP * SY - SR * SP * CY};

#if ENABLE_NAN_DIAGNOSTIC || DO_CHECK
		// Very large inputs can cause NaN's. Want to catch this here
		ensureMsgf(!RotationQuat.ContainsNaN(),
		    TEXT("Invalid input to FRotator::Quaternion - generated NaN output: %s"),
		    *RotationQuat.ToString());
#endif

		return RotationQuat;
	}

	Quat Quat::FromRotatorRad(Rotator rotator)
	{
		return FromRotator(rotator * Math::DEGTORAD);
	}


	Quat InternalBetween(const v3& a, const v3& b, float lengthAB)
	{
		float w = lengthAB + v3::Dot(a, b);
		Quat result;

		if (w >= 1e-6f * lengthAB)
		{
			// Axis = v3::Cross(a, b);
			result = Quat(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x, w);
		}
		else
		{
			// A and B point in opposite directions
			w = 0.f;
			result =
			    Math::Abs(a.x) > Math::Abs(a.y) ? Quat(-a.z, 0.f, a.x, w) : Quat(0.f, -a.z, a.y, w);
		}
		result.Normalize();
		return result;
	}

	Quat Quat::Between(const v3& a, const v3& b)
	{
		const float lengthAB = Math::Sqrt(a.LengthSquared() * b.LengthSquared());
		return InternalBetween(a, b, lengthAB);
	}

	Quat Quat::BetweenNormals(const v3& a, const v3& b)
	{
		return InternalBetween(a, b, 1.f);
	}

	Quat Quat::LookAt(const v3& origin, const v3& dest)
	{
		return Between(origin, dest);
	}


	void Read(Serl::ReadContext& ct, Quat& val)
	{
		ct.BeginObject();
		ct.Next(TX("x"), val.x);
		ct.Next(TX("y"), val.y);
		ct.Next(TX("z"), val.z);
		ct.Next(TX("w"), val.w);
	}
	void Write(Serl::WriteContext& ct, const Quat& val)
	{
		ct.BeginObject();
		ct.Next(TX("x"), val.x);
		ct.Next(TX("y"), val.y);
		ct.Next(TX("z"), val.z);
		ct.Next(TX("w"), val.w);
	}
}    // namespace Rift
