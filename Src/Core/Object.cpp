// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Pipe/Core/Object.h"

namespace p
{
	TPtr<Object> BaseObject::AsPtr() const
	{
		return static_cast<const Object*>(this)->AsPtr();
	}


	TPtr<BaseObject> ObjectOwnership::nextOwner{};

	ObjectOwnership::ObjectOwnership() : self{}, owner{Move(nextOwner)}
	{
		;
	}
	const TPtr<BaseObject>& ObjectOwnership::AsPtr() const
	{
		return self;
	}
	const TPtr<BaseObject>& ObjectOwnership::GetOwner() const
	{
		return owner;
	}

	void Object::ChangeOwner(const TPtr<BaseObject>& inOwner)
	{
		ownership.owner = inOwner;
	}
}    // namespace p