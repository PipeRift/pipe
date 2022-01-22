// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Math/Vector.h"


namespace Rift::Bezier
{
	CORE_API v2 EvaluateCubic(v2 p0, v2 p1, v2 p2, v2 p3, float t);
}    // namespace Rift::Bezier
