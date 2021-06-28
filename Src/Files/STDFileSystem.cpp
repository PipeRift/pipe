// Copyright 2015-2021 Piperift - All rights reserved

#include "Files/Paths.h"
#include "Files/STDFileSystem.h"
#include "Serialization/Contexts.h"


namespace Rift
{
	namespace Serl
	{
		void Read(Serl::ReadContext& ct, Path& value)
		{
			String str;
			ct.Serialize(str);
			value = Paths::FromString(str);
		}

		void Write(Serl::WriteContext& ct, const Path& value)
		{
			ct.Serialize(Paths::ToString(value));
		}
	}    // namespace Serl
}    // namespace Rift
