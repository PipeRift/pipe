// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Memory/STLAllocator.h"
#include "Reflection/ReflectionTypeTraits.h"
#include "Strings/Name.h"
#include "Strings/String.h"

#include <json.hpp>
#include <map>
#include <vector>



namespace Rift
{
	using Json = nlohmann::basic_json<std::map, std::vector, String, bool, i64, u64, double,
	    STLHeapAllocator>;

	DECLARE_REFLECTION_TYPE(Json);
}	 // namespace Rift
