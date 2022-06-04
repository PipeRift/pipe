// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "nanobench.h"

#include <Containers/Array.h>
#include <Containers/Set.h>

using namespace ankerl;
using namespace Pipe;


void RunLookupsBenchmarks()
{
	{
		ankerl::nanobench::Bench lookups;
		constexpr u64 count = 200000;
		lookups.title("Lookups")
		    .performanceCounters(true)
		    .epochs(1)
		    .epochIterations(count)
		    .maxEpochTime(Pipe::Seconds{15});

		{
			TArray<u64> data;
			data.Reserve(count);
			for (u64 i = 0; i < count; ++i)
			{
				data.Add(i);
			}
			u64 i = 0;
			lookups.run("Array - Linear Search", [&data, &i] {
				ankerl::nanobench::doNotOptimizeAway(data.Find(i));
				++i;
			});
		}
		{
			TArray<u64> data;
			data.Reserve(count);
			for (u64 i = 0; i < count; ++i)
			{
				data.Add(i);
			}
			u64 i = 0;
			lookups.run("Array - Binary Search", [&data, &i] {
				ankerl::nanobench::doNotOptimizeAway(data.FindSortedEqual(i));
				++i;
			});
		}
		{
			TSet<u64> data;
			data.Reserve(count);
			for (u64 i = 0; i < count; ++i)
			{
				data.Insert(i);
			}
			u64 i = 0;
			lookups.run("HashSet", [&data, &i] {
				ankerl::nanobench::doNotOptimizeAway(data.Find(i));
				++i;
			});
		}
	}

	/*{
	    ankerl::nanobench::Bench complexityBSearch;
	    complexityBSearch.title("Lookups - HashSet");
	    for (auto size : {64U, 256U, 1024U, 4096U, 16384U, 65536U, 262144U})
	    {
	        TArray<u64> data;
	        data.Reserve(size);
	        for (u64 i = 0; i < size; ++i)
	        {
	            data.Add(i);
	        }
	        u64 i = 0;
	        complexityBSearch.complexityN(size).run("Array - Binary Search", [&data, &i] {
	            data.FindSortedEqual(i);
	            ++i;
	        });
	    }

	    ankerl::nanobench::Bench complexityHashSet;
	    complexityHashSet.title("Lookups - Binary Search");

	    for (auto size : {64U, 256U, 1024U, 4096U, 16384U, 65536U, 262144U})
	    {
	        TSet<u64> data;
	        data.Reserve(size);
	        for (u64 i = 0; i < size; ++i)
	        {
	            data.Insert(i);
	        }
	        u64 i = 0;
	        complexityHashSet.complexityN(size).run("HashSet", [&data, &i] {
	            data.Find(i);
	            ++i;
	        });
	    }
	}*/

	// Give some time to the profiler to send data
	// std::this_thread::sleep_for(std::chrono::seconds(3));
}
