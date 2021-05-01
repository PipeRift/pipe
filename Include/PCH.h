// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

// Includes BEFORE Override.h to avoid conflicts with std::malloc and std::free
#include "Export.h"
#ifdef RiftCore_EXPORTS
#	if defined _WIN32 || defined __CYGWIN__ || defined _MSC_VER
#		define CORE_HIDDEN
#	elif defined __GNUC__ && __GNUC__ >= 4
#		define CORE_HIDDEN __attribute__((visibility("hidden")))
#	else
#		define CORE_HIDDEN
#	endif
#endif


#include "Memory/Override.h"
#include "Platform/Platform.h"

#include <robin_hood.h>

#include <taskflow/taskflow.hpp>
