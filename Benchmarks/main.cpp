// Copyright 2015-2022 Piperift - All rights reserved

#include <Pipe/Memory/NewDelete.h>
P_OVERRIDE_NEW_DELETE

#define ANKERL_NANOBENCH_IMPLEMENT 1
#include "nanobench.h"

// Benches
#include "Arenas.bench.h"
#include "Lookups.bench.h"

int main()
{
	RunArenasBenchmarks();
	RunLookupsBenchmarks();
}
