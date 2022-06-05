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
		CORE_API Context() : Super() {}
		CORE_API Context(const Path& logFolder) : Super(), logFolder{logFolder} {}

		CORE_API void Construct() override
		{
			Super::Construct();

			Log::Init("Saved/Logs");    // Init logger
		}

		CORE_API void BeforeDestroy() override
		{
			Super::BeforeDestroy();
			Log::Shutdown();
		}

		CORE_API TaskSystem& GetTasks()
		{
			return tasks;
		}
	};


	CORE_API TOwnPtr<Context>& GetContextInstance();

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

	CORE_API void ShutdownContext();

	template<typename T = Context>
	TPtr<T> GetContext()
	{
		CheckMsg(
		    GetContextInstance(), "Context is not initialized! Call InitializeContext<Type>().");
		return GetContextInstance().Cast<T>();
	}

}    // namespace p
