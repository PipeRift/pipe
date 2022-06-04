// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"


namespace Pipe::Refl
{
	enum TypeFlags
	{
		Type_NoFlag        = 0,
		Type_NotSerialized = 1 << 0,    // All "NotSerialized" share the same flag value

		Enum_NotSerialized = Type_NotSerialized | (1 << 1),
		Enum_Flags         = 1 << 2,

		Struct_NotSerialized = Type_NotSerialized | (1 << 3),

		Class_NotSerialized = Type_NotSerialized | (1 << 4),
		Class_Abstract      = 1 << 5
	};

	enum PropFlags
	{
		Prop_NoFlag        = 0,
		Prop_NotSerialized = 1 << 0,
		Prop_View          = 1 << 1,
		Prop_Edit          = 1 << 2 | Prop_View,

		// Internal use only
		Prop_Array = 1 << 3    // Assigned automatically when a property is an array
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
}    // namespace Pipe::Refl

namespace Pipe
{
	using namespace Pipe::Refl;
}
