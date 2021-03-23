// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "Platform/Platform.h"

#include <string_view>


namespace Rift
{
	using StringView  = std::basic_string_view<TCHAR, std::char_traits<TCHAR>>;
	using WStringView = std::basic_string_view<WIDECHAR, std::char_traits<WIDECHAR>>;
}    // namespace Rift
