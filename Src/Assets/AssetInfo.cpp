// Copyright 2015-2021 Piperift - All rights reserved

#include "Assets/AssetInfo.h"
#include "Serialization/Contexts.h"


namespace Rift
{
	void AssetInfo::Read(Serl::ReadContext& ct)
	{
		ct.Serialize(id);
	}
	void AssetInfo::Write(Serl::WriteContext& ct) const
	{
		ct.Serialize(id);
	}
}    // namespace Rift
