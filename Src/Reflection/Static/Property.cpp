// Copyright 2015-2021 Piperift - All rights reserved

#include "Reflection/Static/Property.h"


namespace Rift::Refl
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
}    // namespace Rift::Refl
