// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include <functional>


namespace Rift
{
	template <typename Predicate>
	using TFunction = std::function<Predicate>;
}
