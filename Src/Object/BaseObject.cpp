// Copyright 2015-2021 Piperift - All rights reserved

#include "Object/BaseObject.h"

#include "Object/Object.h"
#include "Reflection/Reflection.h"
#include "Reflection/Static/TClass.h"


namespace Rift
{
	Refl::Class* BaseObject::GetType() const
	{
		return static_cast<const Object*>(this)->GetType();
	}

	Ptr<Object> BaseObject::Self() const
	{
		return static_cast<const Object*>(this)->Self();
	}
}    // namespace Rift
