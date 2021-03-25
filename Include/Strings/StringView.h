// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "Platform/Platform.h"

#include <string_view>


namespace Rift
{
	template <typename CharType>
	using TStringView = std::basic_string_view<CharType, std::char_traits<CharType>>;

	using StringView  = TStringView<TChar>;
	using WStringView = TStringView<WideChar>;
}    // namespace Rift
