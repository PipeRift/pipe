// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "CoreObject.h"
#include "Events/Broadcast.h"
#include "Tasks.h"


namespace Rift
{
	class CORE_API Context : public Object
	{
		CLASS(Context, Object)

	private:
		TaskSystem tasks;
		Path logFolder{"Saved/Logs"};


	public:
		Context() : Super() {}
		Context(const Path& logFolder) : Super(), logFolder{logFolder} {}

		void Construct() override
		{
			Super::Construct();

			Log::Init("Saved/Logs");    // Init logger
		}

		void BeforeDestroy() override
		{
			Super::BeforeDestroy();
			Log::Shutdown();
		}

		TaskSystem& GetTasks()
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
