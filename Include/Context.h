// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Assets/AssetManager.h"
#include "Assets/AssetPtr.h"
#include "CoreObject.h"
#include "Events/Broadcast.h"
#include "Tasks.h"


namespace Rift
{
	class CORE_API Context : public Object
	{
		CLASS(Context, Object)

	private:
		TOwnPtr<AssetManager> assetManager;

		TaskSystem tasks;


	public:
		Context() : Super(), assetManager{Create<AssetManager>()} {}

		virtual void Construct() override
		{
			Super::Construct();
			
			Log::Init("Saved/Logs"); // Init logger
			Log::Info("Initialized Context");
		}

		virtual void BeforeDestroy() override
		{
			Super::BeforeDestroy();
			Log::Info("Context has been destroyed");
			Log::Shutdown();
		}

		TPtr<AssetManager> GetAssetManager()
		{
			return assetManager;
		}

		TaskSystem& GetTasks()
		{
			return tasks;
		}
	};


	CORE_API TOwnPtr<Context>& InternalGetContext();

	template <typename T = Context>
	TPtr<T> InitializeContext()
	{
		TOwnPtr<Context>& context = InternalGetContext();
		if (!context)
		{
			context = Create<T>();
		}
		return context.Cast<T>();
	}

	CORE_API void ShutdownContext();

	template <typename T = Context>
	TPtr<T> GetContext()
	{
		assert(
		    InternalGetContext() && "Context is not initialized! Call InitializeContext<Type>().");
		return InternalGetContext().Cast<T>();
	}
}	 // namespace Rift
