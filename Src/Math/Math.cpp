// Copyright 2015-2021 Piperift - All rights reserved

#include "Math/Math.h"

#include "Math/Vector.h"


namespace Rift
{
	void Math::SinCos(float value, float& outSin, float& outCos)
	{
		// Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
		float quotient = (INV_PI * 0.5f) * value;
		if (value >= 0.0f)
		{
			quotient = float(int(quotient + 0.5f));
		}
		else
		{
			quotient = (float)((int)(quotient - 0.5f));
		}
		float y = value - (2.0f * PI) * quotient;

		// Map y to [-pi/2,pi/2] with sin(y) = sin(value).
		float sign;
		if (y > HALF_PI)
		{
			y    = PI - y;
			sign = -1.0f;
		}
		else if (y < -HALF_PI)
		{
			y    = -PI - y;
			sign = -1.0f;
		}
		else
		{
			sign = +1.0f;
		}

		float y2 = y * y;

		// 11-degree minimax approximation
		outSin =
		    (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f)
		             * y2
		         - 0.16666667f)
		            * y2
		        + 1.0f)
		    * y;

		// 10-degree minimax approximation
		float p =
		    ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f)
		            * y2
		        - 0.5f)
		        * y2
		    + 1.0f;
		outCos = sign * p;
	}

	float Math::Atan2(float Y, float X)
	{
		// return atan2f(Y,X);
		// atan2f occasionally returns NaN with perfectly valid input (possibly due to a compiler or
		// library bug). We are replacing it with a minimax approximation with a max relative error
		// of 7.15255737e-007 compared to the C library function. On PC this has been measured to be
		// 2x faster than the std C version.

		const float absX      = Math::Abs(X);
		const float absY      = Math::Abs(Y);
		const bool yAbsBigger = (absY > absX);
		float t0              = yAbsBigger ? absY : absX;    // Max(absY, absX)
		float t1              = yAbsBigger ? absX : absY;    // Min(absX, absY)

		if (t0 == 0.f)
			return 0.f;

		float t3 = t1 / t0;
		float t4 = t3 * t3;

		static const float c[7] = {+7.2128853633444123e-03f, -3.5059680836411644e-02f,
		    +8.1675882859940430e-02f, -1.3374657325451267e-01f, +1.9856563505717162e-01f,
		    -3.3324998579202170e-01f, +1.0f};

		t0 = c[0];
		t0 = t0 * t4 + c[1];
		t0 = t0 * t4 + c[2];
		t0 = t0 * t4 + c[3];
		t0 = t0 * t4 + c[4];
		t0 = t0 * t4 + c[5];
		t0 = t0 * t4 + c[6];
		t3 = t0 * t3;

		t3 = yAbsBigger ? (0.5f * PI) - t3 : t3;
		t3 = (X < 0.0f) ? PI - t3 : t3;
		t3 = (Y < 0.0f) ? -t3 : t3;

		return t3;
	}
}    // namespace Rift
