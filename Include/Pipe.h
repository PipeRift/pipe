// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/StringView.h"
#include "Pipe/Export.h"


namespace p
{
	P_API void Initialize(struct Logger* logger = nullptr);
	P_API void Shutdown();
};    // namespace p
