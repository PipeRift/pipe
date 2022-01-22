// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include <memory>
#include <taskflow/taskflow.hpp>


namespace Rift
{
	using TaskFlow = tf::Taskflow;
	using Task     = tf::Task;
	using Flow     = tf::FlowBuilder;

	using TaskLambda    = std::function<void()>;
	using SubTaskLambda = std::function<void(Flow&)>;
	using ThreadPool    = tf::Executor;

	enum class TaskPool : u8
	{
		Main,
		Workers
	};

	struct TaskSystem
	{
	private:
		// Main thread
		ThreadPool mainPool;
		// Worker threads
		ThreadPool workerPool;


	public:
		CORE_API TaskSystem();

		CORE_API ThreadPool& GetPool(TaskPool pool);

		// Runs a flow in Workers thread pool
		CORE_API std::future<void> Run(TaskFlow& flow, TaskPool pool = TaskPool::Workers)
		{
			return GetPool(pool).run(flow);
		}

		// Creates a flow in Game thread pool
		CORE_API std::future<void> RunMainFlow(TaskFlow& flow)
		{
			return mainPool.run(flow);
		}

		CORE_API u32 GetNumWorkerThreads() const
		{
			return (u32)workerPool.num_workers();
		}

		static CORE_API TaskSystem& Get();
	};
}    // namespace Rift
