// Copyright 2015-2021 Piperift - All rights reserved

#include "Memory/Arenas/GlobalArena.h"


namespace Rift::Memory
{
	static HeapArena globalArena{};


	HeapArena& GetGlobalArena()
	{
		return globalArena;
	}
};
