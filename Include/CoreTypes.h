// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"
#include "Reflection/ReflectionTypeTraits.h"
#include "Strings/Name.h"
#include "Strings/String.h"


namespace Rift
{
	/** Registry new editor-supported types here and on "EngineTypes.cpp" */

	/** To registry a new native type:
	 * 1. Add Definition below
	 * 2. Registry on HandleHelper.cpp
	 * 3. Registry widget creation on PropertyWidget.cpp
	 */
	DECLARE_REFLECTED_TYPE(bool);
	DECLARE_REFLECTED_TYPE(u8);
	DECLARE_REFLECTED_TYPE(i32);
	DECLARE_REFLECTED_TYPE(u32);
	DECLARE_REFLECTED_TYPE(i64);
	DECLARE_REFLECTED_TYPE(u64);
	DECLARE_REFLECTED_TYPE(float);
	DECLARE_REFLECTED_TYPE(double);

	DECLARE_REFLECTED_TYPE(String);
	DECLARE_REFLECTED_TYPE(Name);
}	 // namespace Rift
