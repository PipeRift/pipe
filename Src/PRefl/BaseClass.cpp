// Copyright 2015-2022 Piperift - All rights reserved

#include "PRefl/BaseClass.h"

#include "PRefl/Class.h"
#include "PRefl/ClassType.h"
#include "PRefl/Reflection.h"


namespace p
{
	ClassType* BaseClass::GetType() const
	{
		return static_cast<const Class*>(this)->GetType();
	}

	TPtr<Class> BaseClass::Self() const
	{
		return static_cast<const Class*>(this)->Self();
	}
}    // namespace p
