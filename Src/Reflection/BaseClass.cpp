// Copyright 2015-2022 Piperift - All rights reserved

#include "Reflection/BaseClass.h"

#include "Reflection/Class.h"
#include "Reflection/ClassType.h"
#include "Reflection/Reflection.h"


namespace Pipe::Refl
{
	Refl::ClassType* BaseClass::GetType() const
	{
		return static_cast<const Class*>(this)->GetType();
	}

	TPtr<Class> BaseClass::Self() const
	{
		return static_cast<const Class*>(this)->Self();
	}
}    // namespace Pipe::Refl
