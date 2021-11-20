// Copyright 2015-2021 Piperift - All rights reserved

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
