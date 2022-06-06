// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCore/Platform.h"
#include "PRefl/Class.h"
#include "PRefl/TPropertyHandle.h"


namespace p
{
	// NOTE: Legacy class pending a rework
	class ClassHandle
	{
		Class* instance = nullptr;
		ClassType* type = nullptr;


	public:
		ClassHandle()                   = delete;
		ClassHandle(ClassHandle&&)      = default;
		ClassHandle(const ClassHandle&) = default;
		ClassHandle& operator=(ClassHandle&&) = default;
		ClassHandle& operator=(const ClassHandle&) = default;

		ClassHandle(Class& instance) : instance(*instance) {}
	};
}    // namespace p
