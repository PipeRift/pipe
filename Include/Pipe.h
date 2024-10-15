// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/StringView.h"
#include "Pipe/Export.h"


namespace p
{
	PIPE_API void Initialize(Logger* logger = nullptr);
	PIPE_API void Shutdown();
};    // namespace p
