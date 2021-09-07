// Copyright 2015-2021 Piperift - All rights reserved
// Based on Unreal Engine 4 Crc

#pragma once

#include "Platform/Platform.h"


    /**
	 * CRC hash generation for different types of input data
	 **/
namespace Rift::Crc
{
	/** generates CRC hash of the memory area */
	CORE_API u32 MemCrc32(const void* Data, i32 Length, u32 CRC = 0);
}    // namespace Rift::Crc
