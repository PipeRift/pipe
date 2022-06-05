// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Math/Vector.h"


namespace pipe::Math
{
	CORE_API v2 EvaluateCubicBezier(v2 p0, v2 p1, v2 p2, v2 p3, float t);
}    // namespace pipe::Math

namespace pipe
{
	using namespace pipe::Math;
}
