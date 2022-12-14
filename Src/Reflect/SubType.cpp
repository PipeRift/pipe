// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Reflect/SubType.h"


namespace p
{
	bool SubType::Set(const Type* newType)
	{
		const DataType* baseDataType = Cast<DataType>(base);
		if (!baseDataType)
		{
			type = nullptr;    // Base type is invalid so we reset type
			return false;
		}
		else if (newType && !baseDataType->IsParentOf(Cast<DataType>(newType)))
		{
			return false;
		}
		type = newType;
		return true;
	}
	void SubType::SetBase(const Type* newBase)
	{
		base = newBase;
		Set(type);    // Reassign type, cleaning it if invalid
	}
}    // namespace p
