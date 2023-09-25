// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Math/Time.h"


namespace p
{
	struct PIPE_API FrameTime
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

		// Call before tick
		void PreTick();

		// Call after tick
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
}    // namespace p
