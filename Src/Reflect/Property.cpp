// Copyright 2015-2024 Piperift - All rights reserved

#include "Pipe/Reflect/Property.h"


namespace p
{
	bool Property::HasFlag(PropFlags flag) const
	{
		return HasAnyFlags(flag);
	}
	bool Property::HasAllFlags(PropFlags inFlags) const
	{
		return (flags & inFlags) == inFlags;
	}
	bool Property::HasAnyFlags(PropFlags inFlags) const
	{
		return (flags & inFlags) > 0;
	}
}    // namespace p
