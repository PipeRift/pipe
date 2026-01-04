// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "PipeVectors.h"

#include "PipePlatform.h"


namespace p
{
	float Rotator::ClampAxis(float angle)
	{
		// returns angle in the range (-360,360)
		angle = Mod(angle, 360.f);
		if (angle < 0.f)
		{
			// shift to [0,360) range
			angle += 360.f;
		}
		return angle;
	}

	float Rotator::NormalizeAxis(float angle)
	{
		// returns angle in the range [0,360)
		angle = ClampAxis(angle);
		if (angle > 180.f)
		{
			// shift to (-180,180]
			angle -= 360.f;
		}

		return angle;
	}

	v3 Vectors::GetSafeScaleReciprocal(const v3& scale, float tolerance)
	{
		v3 safeReciprocalScale;
		if (Abs(scale.x) <= tolerance)
		{
			safeReciprocalScale.x = 0.f;
		}
		else
		{
			safeReciprocalScale.x = 1 / scale.x;
		}

		if (Abs(scale.y) <= tolerance)
		{
			safeReciprocalScale.y = 0.f;
		}
		else
		{
			safeReciprocalScale.y = 1 / scale.y;
		}

		if (Abs(scale.z) <= tolerance)
		{
			safeReciprocalScale.z = 0.f;
		}
		else
		{
			safeReciprocalScale.z = 1 / scale.z;
		}

		return safeReciprocalScale;
	}

	v2 Vectors::ClosestPointInLine(v2 a, v2 b, v2 point)
	{
		const v2 ab = b - a;

		const float dot    = v2::Dot(a - point, ab);
		const float length = ab.LengthSquared();
		const float delta  = Clamp(-dot / length, 0.0f, 1.0f);
		return a + (ab * delta);
	}

	v3 Vectors::ClosestPointInLine(v3 a, v3 b, v3 point)
	{
		const v3 ab = b - a;

		const float dot    = v3::Dot(a - point, ab);
		const float length = ab.LengthSquared();
		const float delta  = Clamp(-dot / length, 0.f, 1.f);
		return a + (ab * delta);
	}

	v2 EvaluateCubicBezier(v2 p0, v2 p1, v2 p2, v2 p3, float t)
	{
		// B(t) = (1-t)**3 p0 + 3(1 - t)**2 t p1 + 3(1-t)t**2 p2 + t**3 p3
		const float u  = 1.0f - t;
		const float b0 = u * u * u;
		const float b1 = 3 * u * u * t;
		const float b2 = 3 * u * t * t;
		const float b3 = t * t * t;
		return {b0 * p0.x + b1 * p1.x + b2 * p2.x + b3 * p3.x,
		    b0 * p0.y + b1 * p1.y + b2 * p2.y + b3 * p3.y};
	}


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

	Quat Quat::Inverse() const
	{
		return {-x, -y, -z, w};
	}

	Rotator Quat::ToRotator() const
	{
		const float SingularityTest = z * x - w * y;
		const float YawY            = 2.f * (w * z + x * y);
		const float YawX            = (1.f - 2.f * (Square(y) + Square(z)));

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
			RotatorFromQuat.Yaw()   = Atan2(YawY, YawX) * radToDeg;
			RotatorFromQuat.Roll() =
			    Rotator::NormalizeAxis(-RotatorFromQuat.Yaw() - (2.f * Atan2(x, w) * radToDeg));
		}
		else if (SingularityTest > SINGULARITY_THRESHOLD)
		{
			RotatorFromQuat.Pitch() = 90.f;
			RotatorFromQuat.Yaw()   = Atan2(YawY, YawX) * radToDeg;
			RotatorFromQuat.Roll() =
			    Rotator::NormalizeAxis(RotatorFromQuat.Yaw() - (2.f * Atan2(x, w) * radToDeg));
		}
		else
		{
			RotatorFromQuat.Pitch() = FastAsin(2.f * (SingularityTest)) * radToDeg;
			RotatorFromQuat.Yaw()   = Atan2(YawY, YawX) * radToDeg;
			RotatorFromQuat.Roll() =
			    Atan2(-2.f * (w * x + y * z), (1.f - 2.f * (Square(x) + Square(y)))) * radToDeg;
		}

		return RotatorFromQuat;
	}

	Rotator Quat::ToRotatorRad() const
	{
		return ToRotator() * degToRad;
	}

	bool Quat::Equals(const Quat& other, float tolerance) const
	{
		return NearlyEqual(x, other.x, tolerance) && NearlyEqual(y, other.y, tolerance)
		    && NearlyEqual(z, other.z, tolerance) && NearlyEqual(w, other.w, tolerance);
	}

	void Quat::Normalize(float tolerance)
	{
		const float lengthSqrt = x * x + y * y + z * z + w * w;
		if (lengthSqrt >= tolerance)
		{
			const float scale = InvSqrt(lengthSqrt);
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
		const float DIVIDE_BY_2 = degToRad / 2.f;
		float SP, SY, SR;
		float CP, CY, CR;

		SinCos(rotator.Pitch() * DIVIDE_BY_2, SP, CP);
		SinCos(rotator.Yaw() * DIVIDE_BY_2, SY, CY);
		SinCos(rotator.Roll() * DIVIDE_BY_2, SR, CR);

		Quat RotationQuat{CR * CP * CY + SR * SP * SY, CR * SP * SY - SR * CP * CY,
		    -CR * SP * CY - SR * CP * SY, CR * CP * SY - SR * SP * CY};
		return RotationQuat;
	}

	Quat Quat::FromRotatorRad(Rotator rotator)
	{
		return FromRotator(rotator * degToRad);
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
			w      = 0.f;
			result = Abs(a.x) > Abs(a.y) ? Quat(-a.z, 0.f, a.x, w) : Quat(0.f, -a.z, a.y, w);
		}
		result.Normalize();
		return result;
	}

	Quat Quat::Between(const v3& a, const v3& b)
	{
		const float lengthAB = Sqrt(a.LengthSquared() * b.LengthSquared());
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
}    // namespace p
