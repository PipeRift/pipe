// Copyright 2015-2021 Piperift - All rights reserved

#include "Context.h"
#include "Math/Math.h"
#include "Profiler.h"
#include "Strings/String.h"
#include "Tasks.h"

#include <chrono>
#include <common/TracySystem.hpp>
#include <condition_variable>
#include <mutex>


namespace Rift
{
	TaskSystem::TaskSystem()
	{
		// Prefer 1, but don't exceed max threads
		const u32 mainPoolSize = Math::Min(1u, std::thread::hardware_concurrency());
		// Prefer max threads - main threads, but don't go under 1
		const u32 workerPoolSize =
		    Math::Max(1u, std::thread::hardware_concurrency() - mainPoolSize);

		mainPool   = std::make_shared<ThreadPool>(mainPoolSize);
		workerPool = std::make_shared<ThreadPool>(workerPoolSize);

		// Name main thread
		tracy::SetThreadName("Main");


		// Go over all worker threads naming them and making sure no thread is repeated
		TaskFlow flow;
		std::mutex mtx;
		std::condition_variable cv;
		i32 currentWorker = 0;
		flow.for_each_index(0, i32(workerPool->num_workers()), 1,
		    [&mtx, &cv, &currentWorker, workerPoolSize](i32 i) {
			    std::unique_lock<std::mutex> lck(mtx);
			    ++currentWorker;
			    cv.notify_all();
			    while (workerPoolSize != currentWorker)
			    {
				    cv.wait(lck);
			    }
			    {
				    // Name each worker thread in the debugger
				    tracy::SetThreadName(CString::Format("Worker {}", i + 1).c_str());
			    }
		    });
		auto future = RunFlow(flow);
		cv.notify_all();
		future.wait();
	}

	TaskSystem& TaskSystem::Get()
	{
		return Context::Get()->GetTasks();
	}
}    // namespace Rift
