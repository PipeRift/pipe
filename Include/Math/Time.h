// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "CoreTypes.h"
#include "DateTime.h"


namespace pipe::math
{
	struct CORE_API FrameTime
	{
	protected:
		DateTime previousTime;
		DateTime currentTime = DateTime::Now();

		float realDeltaTime = 0.f;
		float deltaTime     = 0.f;
		float timeDilation  = 1.f;

		// Value of 1/FPS_CAP
		float minFrameTime = 0.f;    // Uncapped


	public:
		FrameTime() = default;

		void Tick();

		void PostTick();

		void SetFPSCap(u32 maxFPS)
		{
			minFrameTime = 1.f / maxFPS;
		}

		void SetTimeDilation(float newTimeDilation)
		{
			timeDilation = newTimeDilation;
		}

		float GetDeltaTime() const
		{
			return deltaTime;
		}
	};
}    // namespace pipe::math

namespace pipe
{
	using namespace pipe::math;
}
