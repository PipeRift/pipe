// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "nanobench.h"
using namespace ankerl;

#include <Pipe/Core/Chrono.h>
#include <Pipe/Memory/Alloc.h>
#include <Pipe/Memory/BestFitArena.h>
#include <Pipe/Memory/BigBestFitArena.h>
#include <Pipe/Memory/LinearArena.h>


void RunArenasBenchmarks()
{
	{
		ankerl::nanobench::Bench consecutiveAlloc;
		consecutiveAlloc.title("Consecutive allocations")
		    .performanceCounters(true)
		    .minEpochIterations(50000)
		    .maxEpochTime(p::Seconds{1});

		{
			consecutiveAlloc.relative(true).run("OS (malloc)", [&] {
				ankerl::nanobench::doNotOptimizeAway(p::Alloc(16));
			});
		}

		{
			p::LinearArena arena{100 * 1024 * 1024};
			consecutiveAlloc.run("LinearArena", [&] {
				ankerl::nanobench::doNotOptimizeAway(p::Alloc(arena, 16));
			});
		}

		{
			p::BestFitArena arena{10 * 1024 * 1024};
			consecutiveAlloc.run("BestFitArena", [&] {
				ankerl::nanobench::doNotOptimizeAway(p::Alloc(arena, 16));
			});
		}

		{
			p::BestFitArena arena{10 * 1024 * 1024};
			consecutiveAlloc.run("BigBestFitArena", [&] {
				p::Alloc(arena, 16);
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
			p::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (p::u32 i = 0; i < 50000; ++i)
			{
				allocated.Add(p::Alloc(16));
			}

			p::i32 i = 0;
			consecutiveFree.relative(true).run("OS (malloc)", [&] {
				p::Free(allocated[i], 16);
				++i;
			});
			for (; i < allocated.Size(); ++i)
			{
				p::Free(allocated[i], 16);
			}
		}

		{
			p::BestFitArena arena{100 * 1024 * 1024};
			p::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (p::u32 i = 0; i < 50000; ++i)
			{
				allocated.Add(p::Alloc(arena, 16));
			}

			p::i32 i = 0;
			consecutiveFree.run("BestFitArena", [&] {
				p::Free(arena, allocated[i], 16);
				++i;
			});
			for (; i < allocated.Size(); ++i)
			{
				p::Free(arena, allocated[i], 16);
			}
		}

		{
			p::BigBestFitArena arena{100 * 1024 * 1024};
			p::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (p::u32 i = 0; i < 50000; ++i)
			{
				allocated.Add(p::Alloc(arena, 16));
			}

			p::i32 i = 0;
			consecutiveFree.run("BigBestFitArena", [&] {
				p::Free(arena, allocated[i], 16);
				++i;
			});
			for (; i < allocated.Size(); ++i)
			{
				p::Free(arena, allocated[i], 16);
			}
		}
	}

	{
		ankerl::nanobench::Bench allocSequence;
		allocSequence.title("Alloc/Free sequence")
		    .performanceCounters(true)
		    .minEpochIterations(50000)
		    .maxEpochTime(p::Seconds{1});

		{
			allocSequence.relative(true).run("OS (malloc)", [&] {
				void* p  = p::Alloc(16);
				void* p2 = p::Alloc(21);
				p::Free(p, 16);
				void* p3 = p::Alloc(8);
				p::Free(p3, 8);
				ankerl::nanobench::doNotOptimizeAway(p);
				ankerl::nanobench::doNotOptimizeAway(p2);
				ankerl::nanobench::doNotOptimizeAway(p3);
				p::Free(p2, 21);
			});
		}

		{
			p::BestFitArena arena{100 * 1024 * 1024};
			allocSequence.run("BestFitArena", [&] {
				void* p  = p::Alloc(arena, 16);
				void* p2 = p::Alloc(arena, 21);
				p::Free(arena, p, 16);
				void* p3 = p::Alloc(arena, 8);
				p::Free(arena, p3, 8);
				ankerl::nanobench::doNotOptimizeAway(p);
				ankerl::nanobench::doNotOptimizeAway(p2);
				ankerl::nanobench::doNotOptimizeAway(p3);
				p::Free(arena, p2, 21);
			});
		}

		{
			p::BigBestFitArena arena{100 * 1024 * 1024};
			allocSequence.run("BigBestFitArena", [&] {
				void* p  = p::Alloc(arena, 16);
				void* p2 = p::Alloc(arena, 21);
				p::Free(arena, p, 16);
				void* p3 = p::Alloc(arena, 8);
				p::Free(arena, p3, 8);
				ankerl::nanobench::doNotOptimizeAway(p);
				ankerl::nanobench::doNotOptimizeAway(p2);
				ankerl::nanobench::doNotOptimizeAway(p3);
				p::Free(arena, p2, 21);
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
			p::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (p::u32 i = 0; i < 25000; ++i)
			{
				allocated.Add(p::Alloc(16));
			}

			ankerl::nanobench::Rng rng(122);
			randomSequence.relative(true).run("OS (malloc)", [&] {
				if (rng() & 1U)
				{
					void* ptr = p::Alloc(16);
					ankerl::nanobench::doNotOptimizeAway(ptr);
					allocated.Add(ptr);
				}
				else
				{
					p::u32 index   = rng.bounded(allocated.Size());
					void* toRemove = allocated[index];
					p::Free(toRemove, 16);
					allocated.RemoveAtSwapUnsafe(index);
				}
			});

			for (void* p : allocated)
			{
				p::Free(p, 16);
			}
		}

		{
			p::BestFitArena arena{100 * 1024 * 1024};
			p::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (p::u32 i = 0; i < 25000; ++i)
			{
				allocated.Add(p::Alloc(arena, 16));
			}

			ankerl::nanobench::Rng rng(122);
			randomSequence.run("BestFitArena", [&] {
				if (rng() & 1U)
				{
					void* ptr = p::Alloc(arena, 16);
					ankerl::nanobench::doNotOptimizeAway(ptr);
					allocated.Add(ptr);
				}
				else
				{
					p::u32 index   = rng.bounded(allocated.Size());
					void* toRemove = allocated[index];
					p::Free(arena, toRemove, 16);
					allocated.RemoveAtSwapUnsafe(index);
				}
			});

			for (void* p : allocated)
			{
				p::Free(arena, p, 16);
			}
		}

		{
			p::BigBestFitArena arena{100 * 1024 * 1024};
			p::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (p::u32 i = 0; i < 25000; ++i)
			{
				allocated.Add(p::Alloc(arena, 16));
			}

			ankerl::nanobench::Rng rng(122);
			randomSequence.run("BigBestFitArena", [&] {
				if (rng() & 1U)
				{
					void* ptr = p::Alloc(arena, 16);
					ankerl::nanobench::doNotOptimizeAway(ptr);
					allocated.Add(ptr);
				}
				else
				{
					p::u32 index   = rng.bounded(allocated.Size());
					void* toRemove = allocated[index];
					p::Free(arena, toRemove, 16);
					allocated.RemoveAtSwapUnsafe(index);
				}
			});

			for (void* p : allocated)
			{
				p::Free(arena, p, 16);
			}
		}
	}

	/*{
	    ankerl::nanobench::Bench complexity1;
	    complexity1.title("Complexity BigBestFitArena (Alloc)");
	    for (auto size : {64U, 256U, 1024U, 4096U, 16384U, 65536U, 262144U})
	    {
	        p::BigBestFitArena arena{(16 + 8) * size};
	        complexity1.complexityN(size).run([&] {
	            p::Alloc(arena, 16);
	        });
	    }
	    std::cout << complexity1.complexityBigO() << std::endl;

	    ankerl::nanobench::Bench complexity2;
	    complexity2.title("Complexity BestFitArena (Alloc)");
	    for (auto size : {64U, 256U, 1024U, 4096U, 16384U, 65536U, 262144U})
	    {
	        p::BestFitArena arena{16 * size};
	        complexity2.complexityN(size).run([&] {
	            p::Alloc(arena, 16);
	        });
	    }
	    std::cout << complexity2.complexityBigO() << std::endl;
	}*/

	// Give some time to the profiler to send data
	// std::this_thread::sleep_for(std::chrono::seconds(3));
}
