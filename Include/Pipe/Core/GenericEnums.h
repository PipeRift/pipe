// Copyright 2015-2022 Piperift - All rights reserved

#pragma once


namespace p::core
{
	enum class YesNo : bool
	{
		Yes   = true,
		No    = false,
		True  = true,
		False = false
	};

	inline bool operator!(YesNo v)
	{
		return v == YesNo::No;
	}
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
