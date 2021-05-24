// Copyright 2015-2021 Piperift - All rights reserved

#include "Math/Quaternion.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"
#include "Serialization/Contexts.h"


namespace Rift
{
	v3 Quat::Rotate(const v3& v) const
	{
		const v3 q(x, y, z);
		const v3 t = 2.f * glm::cross(q, v);
		return v + (w * t) + glm::cross(q, t);
	}

	v3 Quat::Unrotate(const v3& v) const
	{
		const v3 q(-x, -y, -z);    // Inverse
		const v3 t = 2.f * glm::cross(q, v);
		return v + (w * t) + glm::cross(q, t);
	}
	v3 Quat::GetForward() const
	{
		return *this * v3::Forward;
	}

	v3 Quat::GetRight() const
	{
		return *this * v3::Right;
	}

	v3 Quat::GetUp() const
	{
		return *this * v3::Up;
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
                                         (1.f - 2.f * (Math::Square(x) + Math::Square(y)))) *
			                         Math::RADTODEG;
		}

		return RotatorFromQuat;
	}

	Rotator Quat::ToRotatorRad() const
	{
		return ToRotator() * Math::DEGTORAD;
	}

	bool Quat::Equals(const Quat& other, float tolerance) const
	{
		return Math::NearlyEqual(x, other.x, tolerance) &&
		       Math::NearlyEqual(y, other.y, tolerance) &&
		       Math::NearlyEqual(z, other.z, tolerance) && Math::NearlyEqual(w, other.w, tolerance);
	}

	Quat Quat::FromRotator(Rotator rotator)
	{
		const float DIVIDE_BY_2 = Math::DEGTORAD / 2.f;
		float SP, SY, SR;
		float CP, CY, CR;

		Math::SinCos(&SP, &CP, rotator.Pitch() * DIVIDE_BY_2);
		Math::SinCos(&SY, &CY, rotator.Yaw() * DIVIDE_BY_2);
		Math::SinCos(&SR, &CR, rotator.Roll() * DIVIDE_BY_2);

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

	Quat Quat::LookAt(const v3& origin, const v3& dest)
	{
		return glm::lookAt(origin, dest, v3::Forward);
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
