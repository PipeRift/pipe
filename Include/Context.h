// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/Broadcast.h"
#include "Reflection/Class.h"
#include "Tasks.h"


namespace p
{
	class Context : public Class
	{
		CLASS(Context, Class)

	private:
		TaskSystem tasks;
		Path logFolder{"Saved/Logs"};


	public:
		PIPE_API Context() : Super() {}
		PIPE_API Context(const Path& logFolder) : Super(), logFolder{logFolder} {}

		PIPE_API void Construct() override
		{
			Super::Construct();

			Log::Init("Saved/Logs");    // Init logger
		}

		PIPE_API void BeforeDestroy() override
		{
			Super::BeforeDestroy();
			Log::Shutdown();
		}

		PIPE_API TaskSystem& GetTasks()
		{
			return tasks;
		}
	};


	PIPE_API TOwnPtr<Context>& GetContextInstance();

	template<typename T = Context>
	TPtr<T> InitializeContext()
	{
		TOwnPtr<Context>& context = GetContextInstance();
		if (!context)
		{
			context = MakeOwned<T>();
			return context.Cast<T>();
		}
		return {};
	}

	PIPE_API void ShutdownContext();

	template<typename T = Context>
	TPtr<T> GetContext()
	{
		CheckMsg(
		    GetContextInstance(), "Context is not initialized! Call InitializeContext<Type>().");
		return GetContextInstance().Cast<T>();
	}

}    // namespace p
