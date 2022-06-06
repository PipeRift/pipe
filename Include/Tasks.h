// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include <memory>
#include <taskflow/taskflow.hpp>


namespace p
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
		PIPE_API TaskSystem();

		PIPE_API ThreadPool& GetPool(TaskPool pool);

		// Runs a flow in Workers thread pool
		PIPE_API std::future<void> Run(TaskFlow& flow, TaskPool pool = TaskPool::Workers)
		{
			return GetPool(pool).run(flow);
		}

		// Creates a flow in Game thread pool
		PIPE_API std::future<void> RunMainFlow(TaskFlow& flow)
		{
			return mainPool.run(flow);
		}

		PIPE_API u32 GetNumWorkerThreads() const
		{
			return (u32)workerPool.num_workers();
		}

		static PIPE_API TaskSystem& Get();
	};
}    // namespace p
