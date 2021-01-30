// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

// Include taskflow BEFORE Override.h so that std::malloc and std::free dont get affected
#include <taskflow/taskflow.hpp>

#include "Export.h"
#include "Memory/Override.h"
#include "Platform/Platform.h"

#define Move(v) std::move(v)
#define MoveTemp(v) std::move(v)

