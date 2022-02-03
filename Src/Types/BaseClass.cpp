// Copyright 2015-2022 Piperift - All rights reserved

#include "Types/BaseClass.h"

#include "Reflection/ClassType.h"
#include "Reflection/Reflection.h"
#include "Types/Class.h"



namespace Rift
{
	Refl::ClassType* BaseClass::GetType() const
	{
		return static_cast<const Class*>(this)->GetType();
	}

	TPtr<Class> BaseClass::Self() const
	{
		return static_cast<const Class*>(this)->Self();
	}
}    // namespace Rift
