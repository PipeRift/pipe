// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "PipePlatform.h"


namespace p
{
	////////////////////////////////
	// FORWARD DECLARATIONS
	//

	template<typename Type>
	struct IArray;

	template<typename Type, u32 InlineCapacity>
	struct TInlineArray;

	template<typename Type>
	using TArray = TInlineArray<Type, 0>;

	template<typename Type>
	using TSmallArray = TInlineArray<Type, 5>;

	template<typename Type>
	struct TView;
};    // namespace p
