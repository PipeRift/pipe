// Copyright 2015-2021 Piperift - All rights reserved

#include "Memory/Allocator.h"
#include "Strings/Name.h"

//#include <rpmalloc/rpmalloc.h>

#include <Tracy.hpp>


namespace Rift
{
	/*void* Alloc(size_t n)
	{
		void* const p = rpmalloc(n);
		TracyAlloc(p, n);
		return p;
	}

	void Free(void* p)
	{
		TracyFree(p);
		rpfree(p);
	}*/

	namespace Memory
	{
		/// Default Arena instance.
		Arena gArena{TX("Global")};
		Arena* GetGlobalArena()
		{
			return &gArena;
		}

		Arena gAssetsArena{TX("Assets")};
		Arena* GetAssetsArena()
		{
			return &gAssetsArena;
		}
	};	  // namespace Memory
}	 // namespace Rift
