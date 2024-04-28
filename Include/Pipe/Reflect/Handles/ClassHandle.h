// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Reflect/Class.h"


namespace p
{
	// NOTE: Legacy class pending a rework
	class ClassHandle
	{
		Class* instance = nullptr;
		ClassType* type = nullptr;


	public:
		ClassHandle()                              = delete;
		ClassHandle(ClassHandle&&)                 = default;
		ClassHandle(const ClassHandle&)            = default;
		ClassHandle& operator=(ClassHandle&&)      = default;
		ClassHandle& operator=(const ClassHandle&) = default;

		ClassHandle(Class& instance) : instance(*instance) {}
	};
}    // namespace p
