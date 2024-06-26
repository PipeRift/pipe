// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "nanobench.h"
using namespace ankerl;

#include <Pipe/Memory/Alloc.h>
#include <Pipe/Memory/BestFitArena.h>
#include <Pipe/Memory/BigBestFitArena.h>
#include <Pipe/Memory/HeapArena.h>
#include <Pipe/Memory/MonoLinearArena.h>
#include <Pipe/Memory/MultiLinearArena.h>
#include <PipeTime.h>


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
			p::HeapArena arenat;
			p::Arena& arena{arenat};
			consecutiveAlloc.run("HeapArena", [&arena] {
				ankerl::nanobench::doNotOptimizeAway(arena.Alloc(16));
			});
		}

		{
			p::MonoLinearArena arenat{100 * p::Memory::MB};
			p::Arena& arena{arenat};
			consecutiveAlloc.run("MonoLinearArena", [&arena] {
				ankerl::nanobench::doNotOptimizeAway(arena.Alloc(16));
			});
		}

		{
			p::MultiLinearArena arenat{};
			p::Arena& arena{arenat};
			consecutiveAlloc.run("MultiLinearArena", [&arena] {
				ankerl::nanobench::doNotOptimizeAway(arena.Alloc(16));
			});
		}

		{
			p::BestFitArena arenat{100 * p::Memory::MB};
			p::Arena& arena{arenat};
			consecutiveAlloc.run("BestFitArena", [&arena] {
				ankerl::nanobench::doNotOptimizeAway(arena.Alloc(16));
			});
		}

		{
			p::BestFitArena arenat{100 * p::Memory::MB};
			p::Arena& arena{arenat};
			consecutiveAlloc.run("BigBestFitArena", [&arena] {
				arena.Alloc(16);
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
			consecutiveFree.relative(true).run("OS (malloc)", [&i, &allocated] {
				p::Free(allocated[i], 16);
				++i;
			});
			for (; i < allocated.Size(); ++i)
			{
				p::Free(allocated[i], 16);
			}
		}

		{
			p::HeapArena arenat;
			p::Arena& arena{arenat};
			p::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (p::u32 i = 0; i < 50000; ++i)
			{
				allocated.Add(arena.Alloc(16));
			}

			p::i32 i = 0;
			consecutiveFree.run("HeapArena", [&arena, &i, &allocated] {
				arena.Free(allocated[i], 16);
				++i;
			});
			for (; i < allocated.Size(); ++i)
			{
				arena.Free(allocated[i], 16);
			}
		}

		{
			p::MonoLinearArena arenat{100 * p::Memory::MB};
			p::Arena& arena{arenat};
			p::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (p::u32 i = 0; i < 50000; ++i)
			{
				allocated.Add(arena.Alloc(16));
			}

			p::i32 i = 0;
			consecutiveFree.run("MonoLinearArena", [&arena, &i, &allocated] {
				arena.Free(allocated[i], 16);
				++i;
			});
			for (; i < allocated.Size(); ++i)
			{
				arena.Free(allocated[i], 16);
			}
		}

		{
			p::MultiLinearArena arenat{};
			p::Arena& arena{arenat};
			p::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (p::u32 i = 0; i < 50000; ++i)
			{
				allocated.Add(arena.Alloc(16));
			}

			p::i32 i = 0;
			consecutiveFree.run("MultiLinearArena", [&arena, &i, &allocated] {
				arena.Free(allocated[i], 16);
				++i;
			});
			for (; i < allocated.Size(); ++i)
			{
				arena.Free(allocated[i], 16);
			}
		}

		{
			p::BestFitArena arenat{100 * p::Memory::MB};
			p::Arena& arena{arenat};
			p::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (p::u32 i = 0; i < 50000; ++i)
			{
				allocated.Add(arena.Alloc(16));
			}

			p::i32 i = 0;
			consecutiveFree.run("BestFitArena", [&arena, &i, &allocated] {
				arena.Free(allocated[i], 16);
				++i;
			});
			for (; i < allocated.Size(); ++i)
			{
				arena.Free(allocated[i], 16);
			}
		}

		{
			p::BigBestFitArena arenat{100 * p::Memory::MB};
			p::Arena& arena{arenat};
			p::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (p::u32 i = 0; i < 50000; ++i)
			{
				allocated.Add(arena.Alloc(16));
			}

			p::i32 i = 0;
			consecutiveFree.run("BigBestFitArena", [&arena, &i, &allocated] {
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
		    .maxEpochTime(p::Seconds{1});

		{
			allocSequence.relative(true).run("OS (malloc)", [] {
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
			p::HeapArena arenat;
			p::Arena& arena{arenat};
			allocSequence.run("HeapArena", [&arena] {
				void* p  = arena.Alloc(16);
				void* p2 = arena.Alloc(21);
				arena.Free(p, 16);
				void* p3 = arena.Alloc(8);
				arena.Free(p3, 8);
				ankerl::nanobench::doNotOptimizeAway(p);
				ankerl::nanobench::doNotOptimizeAway(p2);
				ankerl::nanobench::doNotOptimizeAway(p3);
				arena.Free(p2, 21);
			});
		}

		{
			p::MonoLinearArena arenat;
			p::Arena& arena{arenat};
			allocSequence.run("MonoLinearArena", [&arena] {
				void* p  = arena.Alloc(16);
				void* p2 = arena.Alloc(21);
				arena.Free(p, 16);
				void* p3 = arena.Alloc(8);
				arena.Free(p3, 8);
				ankerl::nanobench::doNotOptimizeAway(p);
				ankerl::nanobench::doNotOptimizeAway(p2);
				ankerl::nanobench::doNotOptimizeAway(p3);
				arena.Free(p2, 21);
			});
		}

		{
			p::MultiLinearArena arenat;
			p::Arena& arena{arenat};
			allocSequence.run("MultiLinearArena", [&arena] {
				void* p  = arena.Alloc(16);
				void* p2 = arena.Alloc(21);
				arena.Free(p, 16);
				void* p3 = arena.Alloc(8);
				arena.Free(p3, 8);
				ankerl::nanobench::doNotOptimizeAway(p);
				ankerl::nanobench::doNotOptimizeAway(p2);
				ankerl::nanobench::doNotOptimizeAway(p3);
				arena.Free(p2, 21);
			});
		}

		{
			p::BestFitArena arenat{100 * p::Memory::MB};
			p::Arena& arena{arenat};
			allocSequence.run("BestFitArena", [&arena] {
				void* p  = arena.Alloc(16);
				void* p2 = arena.Alloc(21);
				arena.Free(p, 16);
				void* p3 = arena.Alloc(8);
				arena.Free(p3, 8);
				ankerl::nanobench::doNotOptimizeAway(p);
				ankerl::nanobench::doNotOptimizeAway(p2);
				ankerl::nanobench::doNotOptimizeAway(p3);
				arena.Free(p2, 21);
			});
		}

		{
			p::BigBestFitArena arenat{100 * p::Memory::MB};
			p::Arena& arena{arenat};
			allocSequence.run("BigBestFitArena", [&arena] {
				void* p  = arena.Alloc(16);
				void* p2 = arena.Alloc(21);
				arena.Free(p, 16);
				void* p3 = arena.Alloc(8);
				arena.Free(p3, 8);
				ankerl::nanobench::doNotOptimizeAway(p);
				ankerl::nanobench::doNotOptimizeAway(p2);
				ankerl::nanobench::doNotOptimizeAway(p3);
				arena.Free(p2, 21);
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
			randomSequence.relative(true).run("OS (malloc)", [&rng, &allocated] {
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
			p::HeapArena arenat;
			p::Arena& arena{arenat};
			p::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (p::u32 i = 0; i < 25000; ++i)
			{
				allocated.Add(arena.Alloc(16));
			}

			ankerl::nanobench::Rng rng(122);
			randomSequence.run("HeapArena", [&arena, &rng, &allocated] {
				if (rng() & 1U)
				{
					void* ptr = arena.Alloc(16);
					ankerl::nanobench::doNotOptimizeAway(ptr);
					allocated.Add(ptr);
				}
				else
				{
					p::u32 index   = rng.bounded(allocated.Size());
					void* toRemove = allocated[index];
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
			p::MonoLinearArena arenat{100 * p::Memory::MB};
			p::Arena& arena{arenat};
			p::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (p::u32 i = 0; i < 25000; ++i)
			{
				allocated.Add(arena.Alloc(16));
			}

			ankerl::nanobench::Rng rng(122);
			randomSequence.run("MonoLinearArena", [&arena, &rng, &allocated] {
				if (rng() & 1U)
				{
					void* ptr = arena.Alloc(16);
					ankerl::nanobench::doNotOptimizeAway(ptr);
					allocated.Add(ptr);
				}
				else
				{
					p::u32 index   = rng.bounded(allocated.Size());
					void* toRemove = allocated[index];
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
			p::MultiLinearArena arenat{};
			p::Arena& arena{arenat};
			p::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (p::u32 i = 0; i < 25000; ++i)
			{
				allocated.Add(arena.Alloc(16));
			}

			ankerl::nanobench::Rng rng(122);
			randomSequence.run("MultiLinearArena", [&arena, &rng, &allocated] {
				if (rng() & 1U)
				{
					void* ptr = arena.Alloc(16);
					ankerl::nanobench::doNotOptimizeAway(ptr);
					allocated.Add(ptr);
				}
				else
				{
					p::u32 index   = rng.bounded(allocated.Size());
					void* toRemove = allocated[index];
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
			p::BestFitArena arenat{100 * p::Memory::MB};
			p::Arena& arena{arenat};
			p::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (p::u32 i = 0; i < 25000; ++i)
			{
				allocated.Add(arena.Alloc(16));
			}

			ankerl::nanobench::Rng rng(122);
			randomSequence.run("BestFitArena", [&arena, &rng, &allocated] {
				if (rng() & 1U)
				{
					void* ptr = arena.Alloc(16);
					ankerl::nanobench::doNotOptimizeAway(ptr);
					allocated.Add(ptr);
				}
				else
				{
					p::u32 index   = rng.bounded(allocated.Size());
					void* toRemove = allocated[index];
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
			p::BigBestFitArena arenat{100 * p::Memory::MB};
			p::Arena& arena{arenat};
			p::TArray<void*> allocated;
			allocated.Reserve(50000);
			for (p::u32 i = 0; i < 25000; ++i)
			{
				allocated.Add(arena.Alloc(16));
			}

			ankerl::nanobench::Rng rng(122);
			randomSequence.run("BigBestFitArena", [&arena, &rng, &allocated] {
				if (rng() & 1U)
				{
					void* ptr = arena.Alloc(16);
					ankerl::nanobench::doNotOptimizeAway(ptr);
					allocated.Add(ptr);
				}
				else
				{
					p::u32 index   = rng.bounded(allocated.Size());
					void* toRemove = allocated[index];
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
	        p::BigBestFitArena arena{(16 + 8) * size};
	        complexity1.complexityN(size).run([&] {
	            arena.Alloc(16);
	        });
	    }
	    std::cout << complexity1.complexityBigO() << std::endl;

	    ankerl::nanobench::Bench complexity2;
	    complexity2.title("Complexity BestFitArena (Alloc)");
	    for (auto size : {64U, 256U, 1024U, 4096U, 16384U, 65536U, 262144U})
	    {
	        p::BestFitArena arena{16 * size};
	        complexity2.complexityN(size).run([&] {
	            arena.Alloc(16);
	        });
	    }
	    std::cout << complexity2.complexityBigO() << std::endl;
	}*/

	// Give some time to the profiler to send data
	// std::this_thread::sleep_for(std::chrono::seconds(3));
}
