// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"


namespace Rift
{
	enum TypeFlags
	{
		Type_NoFlag    = 0,
		Type_Transient = 1 << 0,    // All transients are the same flag

		Struct_Transient = 1 << 0,

		Class_Transient = 1 << 0,
		Class_Abstract  = 1 << 1
	};

	enum PropFlags
	{
		Prop_NoFlag    = 0,
		Prop_Transient = 1 << 0,
		Prop_View      = 1 << 1,
		Prop_Edit      = 1 << 2 | Prop_View    // Edit implies View too
	};


	// Functions used to surround macro flag values making them template safe
	constexpr TypeFlags InitTypeFlags()
	{
		return Type_NoFlag;
	}
	constexpr TypeFlags InitTypeFlags(TypeFlags flags)
	{
		return flags;
	}
	constexpr PropFlags InitPropFlags()
	{
		return Prop_NoFlag;
	}
	constexpr PropFlags InitPropFlags(PropFlags flags)
	{
		return flags;
	}
}    // namespace Rift
