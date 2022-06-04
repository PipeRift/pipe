// Copyright 2015-2022 Piperift - All rights reserved
// Based on Unreal Engine 4 Crc

#pragma once

#include "Core/Platform.h"


/**
 * CRC hash generation for different types of input data
 **/
namespace Pipe::Core::Crc
{
	/** generates CRC hash of the memory area */
	CORE_API u32 MemCrc32(const void* Data, i32 Length, u32 CRC = 0);
}    // namespace Pipe::Core::Crc

namespace Pipe
{
	using namespace Pipe::Core;
}
