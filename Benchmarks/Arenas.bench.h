// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "nanobench.h"
using namespace ankerl;

#include <Memory/Alloc.h>
#include <Memory/Arenas/BestFitArena.h>
#include <Memory/Arenas/BigBestFitArena.h>


void RunArenasBenchmarks()
{
	{
		ankerl::nanobench::Bench consecutiveAlloc;
		consecutiveAlloc.title("Consecutive allocations")
		    .warmup(100)
		    .relative(true)
		    .performanceCounters(true)
		    .minEpochIterations(50000);

		{
			consecutiveAlloc.run("OS (malloc)", [&] {
				Rift::Alloc(16);
			});
		}

		{
			Rift::Memory::BestFitArena arena{10 * 1024 * 1024};
			consecutiveAlloc.run("BestFitArena", [&] {
				arena.Allocate(16);
			});
		}

		{
			Rift::Memory::BestFitArena arena{10 * 1024 * 1024};
			consecutiveAlloc.run("BigBestFitArena", [&] {
				arena.Allocate(16);
			});
		}
	}

	{
		ankerl::nanobench::Bench consecutiveFree;
		consecutiveFree.title("Consecutive free")
		    .warmup(100)
		    .relative(true)
		    .performanceCounters(true)
		    .minEpochIterations(50000);

		{
			consecutiveFree.run("OS (malloc)", [&] {
				Rift::Alloc(16);
			});
		}

		{
			Rift::Memory::BestFitArena arena{10 * 1024 * 1024};
			consecutiveFree.run("BestFitArena", [&] {
				arena.Allocate(16);
			});
		}

		{
			Rift::Memory::BigBestFitArena arena{10 * 1024 * 1024};
			consecutiveFree.run("BigBestFitArena", [&] {
				arena.Allocate(16);
			});
		}
	}

	{
		ankerl::nanobench::Bench allocSequence;
		allocSequence.title("Alloc/Free sequence")
		    .warmup(100)
		    .relative(true)
		    .performanceCounters(true)
		    .minEpochIterations(50000);

		{
			allocSequence.run("OS (malloc)", [&] {
				void* p  = Rift::Alloc(16);
				void* p2 = Rift::Alloc(21);
				Rift::Free(p);
				void* p3 = Rift::Alloc(8);
				Rift::Free(p3);
				Rift::Free(p2);
			});
		}

		{
			Rift::Memory::BestFitArena arena{10 * 1024 * 1024};
			allocSequence.run("BestFitArena", [&] {
				void* p  = arena.Allocate(16);
				void* p2 = arena.Allocate(21);
				arena.Free(p, 16);
				void* p3 = arena.Allocate(8);
				arena.Free(p3, 8);
				arena.Free(p2, 21);
			});
		}

		{
			Rift::Memory::BigBestFitArena arena{10 * 1024 * 1024};
			allocSequence.run("BigBestFitArena", [&] {
				void* p  = arena.Allocate(16);
				void* p2 = arena.Allocate(21);
				arena.Free(p, 16);
				void* p3 = arena.Allocate(8);
				arena.Free(p3, 8);
				arena.Free(p2, 21);
			});
		}
	}

	/*{
	    ankerl::nanobench::Bench complexity1;
	    complexity1.title("Complexity BigBestFitArena (Alloc)");
	    for (auto size : {64U, 256U, 1024U, 4096U, 16384U, 65536U, 262144U})
	    {
	        Rift::Memory::BigBestFitArena arena{(16 + 8) * size};
	        complexity1.complexityN(size).run([&] {
	            arena.Allocate(16);
	        });
	    }
	    std::cout << complexity1.complexityBigO() << std::endl;

	    ankerl::nanobench::Bench complexity2;
	    complexity2.title("Complexity BestFitArena (Alloc)");
	    for (auto size : {64U, 256U, 1024U, 4096U, 16384U, 65536U, 262144U})
	    {
	        Rift::Memory::BestFitArena arena{16 * size};
	        complexity2.complexityN(size).run([&] {
	            arena.Allocate(16);
	        });
	    }
	    std::cout << complexity2.complexityBigO() << std::endl;
	}*/
}
