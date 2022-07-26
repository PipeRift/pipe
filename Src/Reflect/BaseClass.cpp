// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Reflect/BaseClass.h"

#include "Pipe/Reflect/Class.h"
#include "Pipe/Reflect/ClassType.h"
#include "Pipe/Reflect/Reflection.h"


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


	TPtr<BaseClass> ClassOwnership::nextOwner{};
}    // namespace p
