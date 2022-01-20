// Copyright 2015-2021 Piperift - All rights reserved

#include "Types/BaseClass.h"

#include "Reflection/Reflection.h"
#include "Reflection/Static/ClassType.h"
#include "Types/Class.h"


namespace Rift
{
	Refl::ClassType* BaseClass::GetClass() const
	{
		return static_cast<const Class*>(this)->GetClass();
	}

	TPtr<Class> BaseClass::Self() const
	{
		return static_cast<const Class*>(this)->Self();
	}
}    // namespace Rift
