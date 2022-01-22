// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "CoreTypes.h"
#include "Reflection/Class.h"
#include "Reflection/TPropertyHandle.h"


namespace Rift::Refl
{
	class ClassHandle
	{
		const TPtr<BaseClass> instance;

	public:
		ClassHandle()                   = delete;
		ClassHandle(ClassHandle&&)      = default;
		ClassHandle(const ClassHandle&) = default;
		ClassHandle& operator=(ClassHandle&&) = default;
		ClassHandle& operator=(const ClassHandle&) = default;
		virtual ~ClassHandle()                     = default;

		ClassHandle(const TPtr<BaseClass>& instance) : instance(instance) {}
	};
}    // namespace Rift::Refl
