// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Broadcast.h"
#include "Pipe/Reflect/Class.h"


namespace p
{
	class Context : public Class
	{
		CLASS(Context, Class)

	private:
		Path logFolder{"Saved/Logs"};


	public:
		PIPE_API Context() : Super()
		{
			Log::Init("Saved/Logs");    // Init logger
		}
		PIPE_API Context(const Path& logFolder) : logFolder{logFolder}
		{
			Log::Init("Saved/Logs");    // Init logger
		}

		PIPE_API ~Context() override
		{
			Log::Shutdown();
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
