// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"
#include "Serialization/SerializationTypes.h"
#include "TypeTraits.h"


namespace Rift::Serl
{
	struct IFormatReader
	{};

	struct IFormatWriter
	{};


	template <Format format>
	struct FormatBind
	{
		static constexpr bool available = false;
		using Reader                    = void;
		using Writer                    = void;
	};

	template <Format format>
	concept HasReader = !IsVoid<typename FormatBind<format>::Reader>;

	template <Format format>
	concept HasWriter = !IsVoid<typename FormatBind<format>::Writer>;
}    // namespace Rift::Serl
