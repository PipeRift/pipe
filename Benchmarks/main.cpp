// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeNewDelete.h>
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
