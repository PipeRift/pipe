// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "nanobench.h"
using namespace ankerl;

#include <Memory/Alloc.h>
#include <Memory/Arenas/BestFitArena.h>
#include <Memory/Arenas/BigBestFitArena.h>
#include <Memory/Arenas/LinearArena.h>
#include <Misc/Chrono.h>


void RunArenasBenchmarks()
{
	{
		ankerl::nanobench::Bench consecutiveAlloc;
		consecutiveAlloc.title("Consecutive allocations")
		    .performanceCounters(true)
		    .minEpochIterations(50000)
		    .maxEpochTime(Pipe::Seconds{1});

		{
			consecutiveAlloc.relative(true).run("OS (malloc)", [&] {
				ankerl::nanobench::doNotOptimizeAway(Pipe::Alloc(16));
			});
		}

		{
			Pipe::Memory::LinearArena arena{100 * 1024 * 1024};
			consecutiveAlloc.run("LinearArena", [&] {
				ankerl::nanobench::doNotOptimizeAway(arena.Allocate(16));
			});
		}

		{
			Pipe::Memory::BestFitArena arena{10 * 1024 * 1024};
			consecutiveAlloc.run("BestFitArena", [&] {
				ankerl::nanobench::doNotOptimizeAway(arena.Allocate(16));
			});
		}

		{
			Pipe::Memory::BestFitArena arena{10 * 1024 * 1024};
			consecutiveAlloc.run("BigBestFitArena", [&] {
				arena.Allocate(16);
			});
		}
	}

	{
		ankerl::nanobench::Bench consecutiveFree;
		consecutiveFree.title("Consecutive free")
		    .performanceCounters(true)
		    .epochs(1)
		    .epochIterations(50000);

		{
			Pipe::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (Pipe::u32 i = 0; i < 50000; ++i)
			{
				allocated.Add(Pipe::Alloc(16));
			}

			Pipe::i32 i = 0;
			consecutiveFree.relative(true).run("OS (malloc)", [&] {
				Pipe::Free(allocated[i]);
				++i;
			});
			for (; i < allocated.Size(); ++i)
			{
				Pipe::Free(allocated[i]);
			}
		}

		{
			Pipe::Memory::BestFitArena arena{100 * 1024 * 1024};
			Pipe::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (Pipe::u32 i = 0; i < 50000; ++i)
			{
				allocated.Add(arena.Allocate(16));
			}

			Pipe::i32 i = 0;
			consecutiveFree.run("BestFitArena", [&] {
				arena.Free(allocated[i], 16);
				++i;
			});
			for (; i < allocated.Size(); ++i)
			{
				arena.Free(allocated[i], 16);
			}
		}

		{
			Pipe::Memory::BigBestFitArena arena{100 * 1024 * 1024};
			Pipe::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (Pipe::u32 i = 0; i < 50000; ++i)
			{
				allocated.Add(arena.Allocate(16));
			}

			Pipe::i32 i = 0;
			consecutiveFree.run("BigBestFitArena", [&] {
				arena.Free(allocated[i], 16);
				++i;
			});
			for (; i < allocated.Size(); ++i)
			{
				arena.Free(allocated[i], 16);
			}
		}
	}

	{
		ankerl::nanobench::Bench allocSequence;
		allocSequence.title("Alloc/Free sequence")
		    .performanceCounters(true)
		    .minEpochIterations(50000)
		    .maxEpochTime(Pipe::Seconds{1});

		{
			allocSequence.relative(true).run("OS (malloc)", [&] {
				void* p  = Pipe::Alloc(16);
				void* p2 = Pipe::Alloc(21);
				Pipe::Free(p);
				void* p3 = Pipe::Alloc(8);
				Pipe::Free(p3);
				ankerl::nanobench::doNotOptimizeAway(p);
				ankerl::nanobench::doNotOptimizeAway(p2);
				ankerl::nanobench::doNotOptimizeAway(p3);
			});
		}

		{
			Pipe::Memory::BestFitArena arena{100 * 1024 * 1024};
			allocSequence.run("BestFitArena", [&] {
				void* p  = arena.Allocate(16);
				void* p2 = arena.Allocate(21);
				arena.Free(p, 16);
				void* p3 = arena.Allocate(8);
				arena.Free(p3, 8);
				ankerl::nanobench::doNotOptimizeAway(p);
				ankerl::nanobench::doNotOptimizeAway(p2);
				ankerl::nanobench::doNotOptimizeAway(p3);
			});
		}

		{
			Pipe::Memory::BigBestFitArena arena{100 * 1024 * 1024};
			allocSequence.run("BigBestFitArena", [&] {
				void* p  = arena.Allocate(16);
				void* p2 = arena.Allocate(21);
				arena.Free(p, 16);
				void* p3 = arena.Allocate(8);
				arena.Free(p3, 8);
				ankerl::nanobench::doNotOptimizeAway(p);
				ankerl::nanobench::doNotOptimizeAway(p2);
				ankerl::nanobench::doNotOptimizeAway(p3);
			});
		}
	}

	{
		ankerl::nanobench::Bench randomSequence;
		randomSequence.title("Alloc/Free random sequence")
		    .performanceCounters(true)
		    .epochs(1)
		    .epochIterations(50000);

		{
			Pipe::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (Pipe::u32 i = 0; i < 25000; ++i)
			{
				allocated.Add(Pipe::Alloc(16));
			}

			ankerl::nanobench::Rng rng(122);
			randomSequence.relative(true).run("OS (malloc)", [&] {
				if (rng() & 1U)
				{
					void* ptr = Pipe::Alloc(16);
					ankerl::nanobench::doNotOptimizeAway(ptr);
					allocated.Add(ptr);
				}
				else
				{
					Pipe::u32 index = rng.bounded(allocated.Size());
					void* toRemove  = allocated[index];
					Pipe::Free(toRemove);
					allocated.RemoveAtSwapUnsafe(index);
				}
			});

			for (void* p : allocated)
			{
				Pipe::Free(p);
			}
		}

		{
			Pipe::Memory::BestFitArena arena{100 * 1024 * 1024};
			Pipe::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (Pipe::u32 i = 0; i < 25000; ++i)
			{
				allocated.Add(arena.Allocate(16));
			}

			ankerl::nanobench::Rng rng(122);
			randomSequence.run("BestFitArena", [&] {
				if (rng() & 1U)
				{
					void* ptr = arena.Allocate(16);
					ankerl::nanobench::doNotOptimizeAway(ptr);
					allocated.Add(ptr);
				}
				else
				{
					Pipe::u32 index = rng.bounded(allocated.Size());
					void* toRemove  = allocated[index];
					arena.Free(toRemove, 16);
					allocated.RemoveAtSwapUnsafe(index);
				}
			});

			for (void* p : allocated)
			{
				arena.Free(p, 16);
			}
		}

		{
			Pipe::Memory::BigBestFitArena arena{100 * 1024 * 1024};
			Pipe::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (Pipe::u32 i = 0; i < 25000; ++i)
			{
				allocated.Add(arena.Allocate(16));
			}

			ankerl::nanobench::Rng rng(122);
			randomSequence.run("BigBestFitArena", [&] {
				if (rng() & 1U)
				{
					void* ptr = arena.Allocate(16);
					ankerl::nanobench::doNotOptimizeAway(ptr);
					allocated.Add(ptr);
				}
				else
				{
					Pipe::u32 index = rng.bounded(allocated.Size());
					void* toRemove  = allocated[index];
					arena.Free(toRemove, 16);
					allocated.RemoveAtSwapUnsafe(index);
				}
			});

			for (void* p : allocated)
			{
				arena.Free(p, 16);
			}
		}
	}

	/*{
	    ankerl::nanobench::Bench complexity1;
	    complexity1.title("Complexity BigBestFitArena (Alloc)");
	    for (auto size : {64U, 256U, 1024U, 4096U, 16384U, 65536U, 262144U})
	    {
	        Pipe::Memory::BigBestFitArena arena{(16 + 8) * size};
	        complexity1.complexityN(size).run([&] {
	            arena.Allocate(16);
	        });
	    }
	    std::cout << complexity1.complexityBigO() << std::endl;

	    ankerl::nanobench::Bench complexity2;
	    complexity2.title("Complexity BestFitArena (Alloc)");
	    for (auto size : {64U, 256U, 1024U, 4096U, 16384U, 65536U, 262144U})
	    {
	        Pipe::Memory::BestFitArena arena{16 * size};
	        complexity2.complexityN(size).run([&] {
	            arena.Allocate(16);
	        });
	    }
	    std::cout << complexity2.complexityBigO() << std::endl;
	}*/

	// Give some time to the profiler to send data
	// std::this_thread::sleep_for(std::chrono::seconds(3));
}
