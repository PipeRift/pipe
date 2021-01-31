// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

// Includes BEFORE Override.h to avoid conflicts with std::malloc and std::free
#include <robin_hood.h>
#include <taskflow/taskflow.hpp>

#include "Export.h"
#include "Memory/Override.h"
#include "Platform/Platform.h"


#define Move(v) std::move(v)
#define MoveTemp(v) std::move(v)

