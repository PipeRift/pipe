// Copyright 2015-2021 Piperift - All rights reserved

#include "Memory/Arenas/GlobalArena.h"


namespace Rift::Memory
{
	static BigBestFitArena globalArena{1024 * 1024};    // 1MB initial block size


	BigBestFitArena& GetGlobalArena()
	{
		return globalArena;
	}
};    // namespace Rift::Memory
