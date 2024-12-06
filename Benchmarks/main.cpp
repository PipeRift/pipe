// Copyright 2015-2024 Piperift - All rights reserved

#include <Pipe/NewDelete.h>
//  Override as first include

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
