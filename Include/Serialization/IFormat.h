// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"
#include "Serialization/SerializationTypes.h"
#include "Strings/StringView.h"


namespace Rift::Serl
{
	enum Format
	{
		Format_None = 0,
		Format_Json,
		Format_Bson,
		Format_Binary
	};

	// Readers transform serialized fields into different data formats
	struct IFormatReader
	{};

	template <Format format>
	struct FormatBind
	{
		using Reader = void;
	};
}    // namespace Rift::Serl
