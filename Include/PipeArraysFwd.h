// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "PipePlatform.h"


#ifndef P_ARRAY_DEFAULT_INLINECAPACITY
	#define P_ARRAY_DEFAULT_INLINECAPACITY 0
#endif

namespace p
{
	////////////////////////////////
	// FORWARD DECLARATIONS
	//

	template<typename Type>
	struct IArray;

	template<typename Type, u32 InlineCapacity = P_ARRAY_DEFAULT_INLINECAPACITY>
	struct TArray;

	template<typename Type>
	using TSmallArray = TArray<Type, 5>;

	template<typename Type>
	struct TView;
};    // namespace p
