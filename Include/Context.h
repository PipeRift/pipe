// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "CoreObject.h"
#include "Events/Broadcast.h"
#include "Tasks.h"


namespace Rift
{
	class Context : public Object
	{
		CLASS(Context, Object)

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

	template <typename T = Context>
	bool InitializeContext()
	{
		TOwnPtr<Context>& context = GetContextInstance();
		if (!context)
		{
			context = MakeOwned<T>();
			return context.IsValid();
		}
		return false;
	}

	CORE_API void ShutdownContext();

	template <typename T = Context>
	TPtr<T> GetContext()
	{
		CheckMsg(
		    GetContextInstance(), "Context is not initialized! Call InitializeContext<Type>().");
		return GetContextInstance().Cast<T>();
	}

}    // namespace Rift
