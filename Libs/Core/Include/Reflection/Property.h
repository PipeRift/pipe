// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Object/BaseObject.h"
#include "Object/BaseStruct.h"
#include "ReflectionTags.h"
#include "Strings/Name.h"
#include "Strings/String.h"

#include <EASTL/shared_ptr.h>


class BaseType;
struct PropertyHandle;

/**
 * Static information about a property
 */
class Property
{
private:
	BaseType* typePtr;
	Name typeName;
	Name name;
	String displayName;
	ReflectionTags tags;


	Property() = delete;
	Property(Property&&) = delete;
	Property(const Property&) = delete;

protected:
	Property(BaseType* typePtr, Name typeName, Name name, ReflectionTags tags)
		: typeName(typeName), typePtr(typePtr), name(name), tags(tags)
	{
		SetDisplayName(name.ToString());
	}

public:
	virtual ~Property() = default;

	const String& GetName() const
	{
		return name.ToString();
	}
	const String& GetDisplayName() const
	{
		return displayName;
	}

	bool HasTag(ReflectionTags tag) const
	{
		return HasAnyTags(tag);
	}
	bool HasAllTags(ReflectionTags inTags) const
	{
		return (tags & inTags) == inTags;
	}
	bool HasAnyTags(ReflectionTags inTags) const
	{
		return (tags & inTags) > 0;
	}

	BaseType* GetContainerType() const
	{
		return typePtr;
	}
	Name GetTypeName() const
	{
		return typeName;
	}

	virtual eastl::shared_ptr<PropertyHandle> CreateHandle(const Ptr<BaseObject>& instance) const = 0;
	virtual eastl::shared_ptr<PropertyHandle> CreateHandle(BaseStruct* instance) const = 0;

private:
	void SetDisplayName(const String& inDisplayName);
};