// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include <memory>


namespace Rift
{
	using TaskFlow = tf::Taskflow;
	using Task = tf::Task;
	using Flow = tf::FlowBuilder;

	using TaskLambda = std::function<void()>;
	using SubTaskLambda = std::function<void(Flow&)>;


	struct CORE_API TaskSystem
	{
		using ThreadPool = tf::Executor;

	private:
		// Main thread
		std::shared_ptr<ThreadPool> mainPool;
		// Worker threads
		std::shared_ptr<ThreadPool> workerPool;


	public:
		TaskSystem();

		// Runs a flow in Workers thread pool
		std::future<void> RunFlow(TaskFlow& flow) const
		{
			return workerPool->run(flow);
		}

		// Creates a flow in Game thread pool
		std::future<void> RunMainFlow(TaskFlow& flow) const
		{
			return mainPool->run(flow);
		}

		u32 GetNumWorkerThreads() const
		{
			return (u32) workerPool->num_workers();
		}

		static TaskSystem& Get();
	};
}	 // namespace Rift
