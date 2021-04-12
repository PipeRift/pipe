// Copyright 2015-2021 Piperift - All rights reserved

#include "Serialization/Archive.h"


namespace Rift
{
	void Archive::Serialize(StringView name, v2& val)
	{
		BeginObject(name);
		{
			Serialize(TX("x"), val.x);
			Serialize(TX("y"), val.y);
		}
		EndObject();
	}

	void Archive::Serialize(StringView name, v2_u32& val)
	{
		BeginObject(name);
		{
			Serialize(TX("x"), val.x);
			Serialize(TX("y"), val.y);
		}
		EndObject();
	}

	void Archive::Serialize(StringView name, v3& val)
	{
		BeginObject(name);
		{
			Serialize(TX("x"), val.x);
			Serialize(TX("y"), val.y);
			Serialize(TX("z"), val.z);
		}
		EndObject();
	}

	void Archive::Serialize(StringView name, Quat& val)
	{
		BeginObject(name);
		{
			Serialize(TX("x"), val.x);
			Serialize(TX("y"), val.y);
			Serialize(TX("z"), val.z);
			Serialize(TX("w"), val.w);
		}
		EndObject();
	}
}    // namespace Rift
