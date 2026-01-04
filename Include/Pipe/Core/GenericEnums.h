// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once


namespace p
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
}    // namespace p
