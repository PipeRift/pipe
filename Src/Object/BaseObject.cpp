// Copyright 2015-2021 Piperift - All rights reserved

#include "Object/BaseObject.h"

#include "Object/Object.h"
#include "Reflection/Reflection.h"
#include "Reflection/Static/ClassType.h"


namespace Rift
{
	Refl::ClassType* BaseObject::GetClass() const
	{
		return static_cast<const Object*>(this)->GetClass();
	}

	TPtr<Object> BaseObject::Self() const
	{
		return static_cast<const Object*>(this)->Self();
	}
}    // namespace Rift
