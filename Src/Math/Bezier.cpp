// Copyright 2015-2022 Piperift - All rights reserved

#include "Math/Bezier.h"


namespace p::math
{
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
}    // namespace p::math
