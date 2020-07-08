// Copyright 2015-2020 Piperift - All rights reserved

#include "Reflection/Property.h"

#include "Reflection/Runtime/PropertyHandle.h"


void Property::SetDisplayName(const String& inDisplayName)
{
	displayName = inDisplayName;

	// Remove bX preffix
	if (displayName.size() > 1 && CString::StartsWith(displayName, "b") && isupper(displayName[1]))
	{
		CString::RemoveFromStart(displayName, 1);
		displayName[0] = eastl::CharToLower(displayName[0]);
	}
}
