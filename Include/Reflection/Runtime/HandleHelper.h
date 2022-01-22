// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "CoreTypes.h"
#include "Reflection/Runtime/ClassHandle.h"
#include "Types/BaseClass.h"


namespace Rift::Refl
{
	class HandleHelper
	{
	public:
		// #TODO: Move to TClass
		static std::shared_ptr<ClassHandle> CreateClassHandle(const TPtr<BaseClass>& instance)
		{
			return std::make_shared<ClassHandle>(instance);
		}
	};
}    // namespace Rift::Refl
