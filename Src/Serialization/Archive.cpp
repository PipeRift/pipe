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

	void JsonArchive::Serialize(StringView name, bool& val)
	{
		if (IsLoading())
		{
			const Json& field = Data()[name.data()];
			if (field.is_boolean())
			{
				val = field.get<bool>();
			}
		}
		else
		{
			Data()[name.data()] = val;
		}
	}

	void JsonArchive::Serialize(StringView name, u8& val)
	{
		if (IsLoading())
		{
			const Json& field = Data()[name.data()];
			if (field.is_number_unsigned())
			{
				val = field.get<u8>();
			}
		}
		else
		{
			Data()[name.data()] = val;
		}
	}

	void JsonArchive::Serialize(StringView name, i32& val)
	{
		if (IsLoading())
		{
			const Json& field = Data()[name.data()];
			if (field.is_number_unsigned())
			{
				val = field.get<i32>();
			}
		}
		else
		{
			Data()[name.data()] = val;
		}
	}

	void JsonArchive::Serialize(StringView name, u32& val)
	{
		if (IsLoading())
		{
			const Json& field = Data()[name.data()];
			if (field.is_number_unsigned())
			{
				val = field.get<u32>();
			}
		}
		else
		{
			Data()[name.data()] = val;
		}
	}

	void JsonArchive::Serialize(StringView name, float& val)
	{
		if (IsLoading())
		{
			const Json& field = Data()[name.data()];
			if (field.is_number())
			{
				val = field.get<float>();
			}
		}
		else
		{
			Data()[name.data()] = val;
		}
	}

	void JsonArchive::Serialize(StringView name, String& val)
	{
		if (IsLoading())
		{
			const Json& field = Data()[name.data()];
			if (field.is_string())
			{
				val = field.get<String>();
			}
		}
		else
		{
			Data()[name.data()] = val;
		}
	}

	void JsonArchive::Serialize(StringView name, Json& val)
	{
		if (IsLoading())
		{
			val = Data()[name.data()];
		}
		else
		{
			Data()[name.data()] = val;
		}
	}
}	 // namespace Rift
