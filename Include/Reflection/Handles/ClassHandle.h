// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "CoreTypes.h"
#include "Reflection/Class.h"
#include "Reflection/TPropertyHandle.h"


namespace Rift::Refl
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
}    // namespace Rift::Refl