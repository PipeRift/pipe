// Copyright 2015-2022 Piperift - All rights reserved

#include "PMath/Vector.h"

#include "PCore/PlatformMisc.h"
#include "PMath/Rotator.h"
#include "PSerl/Serialization.h"



namespace p
{
	float Rotator::ClampAxis(float angle)
	{
		// returns angle in the range (-360,360)
		angle = math::Mod(angle, 360.f);
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


	void Read(Reader& ct, v2& val)
	{
		ct.BeginObject();
		ct.Next(TX("x"), val.x);
		ct.Next(TX("y"), val.y);
	}
	void Write(Writer& ct, v2 val)
	{
		ct.BeginObject();
		ct.Next(TX("x"), val.x);
		ct.Next(TX("y"), val.y);
	}

	void Read(Reader& ct, v2_u32& val)
	{
		ct.BeginObject();
		ct.Next(TX("x"), val.x);
		ct.Next(TX("y"), val.y);
	}
	void Write(Writer& ct, v2_u32 val)
	{
		ct.BeginObject();
		ct.Next(TX("x"), val.x);
		ct.Next(TX("y"), val.y);
	}

	void Read(Reader& ct, v3& val)
	{
		ct.BeginObject();
		ct.Next(TX("x"), val.x);
		ct.Next(TX("y"), val.y);
		ct.Next(TX("z"), val.z);
	}
	void Write(Writer& ct, const v3& val)
	{
		ct.BeginObject();
		ct.Next(TX("x"), val.x);
		ct.Next(TX("y"), val.y);
		ct.Next(TX("z"), val.z);
	}

	v3 Vectors::GetSafeScaleReciprocal(const v3& scale, float tolerance)
	{
		v3 safeReciprocalScale;
		if (math::Abs(scale.x) <= tolerance)
			safeReciprocalScale.x = 0.f;
		else
			safeReciprocalScale.x = 1 / scale.x;

		if (math::Abs(scale.y) <= tolerance)
			safeReciprocalScale.y = 0.f;
		else
			safeReciprocalScale.y = 1 / scale.y;

		if (math::Abs(scale.z) <= tolerance)
			safeReciprocalScale.z = 0.f;
		else
			safeReciprocalScale.z = 1 / scale.z;

		return safeReciprocalScale;
	}

	v2 Vectors::ClosestPointInLine(v2 a, v2 b, v2 point)
	{
		const v2 ab = b - a;

		const float dot    = v2::Dot(a - point, ab);
		const float length = ab.LengthSquared();
		const float delta  = math::Clamp(-dot / length, 0.0f, 1.0f);
		return a + (ab * delta);
	}

	v3 Vectors::ClosestPointInLine(v3 a, v3 b, v3 point)
	{
		const v3 ab = b - a;

		const float dot    = v3::Dot(a - point, ab);
		const float length = ab.LengthSquared();
		const float delta  = math::Clamp(-dot / length, 0.f, 1.f);
		return a + (ab * delta);
	}
}    // namespace p
