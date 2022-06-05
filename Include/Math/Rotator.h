// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Math/Vector.h"


namespace pipe::Math
{
	struct Rotator : public v3
	{
		Rotator() {}
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
}    // namespace pipe::Math

namespace pipe
{
	using namespace pipe::Math;
}
