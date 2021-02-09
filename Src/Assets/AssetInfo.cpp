// Copyright 2015-2021 Piperift - All rights reserved
#include "Assets/AssetInfo.h"

#include "Serialization/Archive.h"


namespace Rift
{
	bool AssetInfo::Serialize(class Archive& ar, const char* name)
	{
		ar(name, id);
		return true;
	}
}	 // namespace Rift
