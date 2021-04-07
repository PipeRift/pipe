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
		static ObjectPtr<Context> globalInstance;

		ObjectPtr<AssetManager> assetManager;

		TaskSystem tasks;


	public:
		/** Called to initialize the global context. */
		static void Initialize()
		{
			if (!globalInstance)
			{
				globalInstance = Create<Context>();
			}
		}

		/** Called to manually shutdown the global context. */
		static void Shutdown()
		{
			if (globalInstance)
			{
				globalInstance.Release();
			}
		}

		Context() : Super(), assetManager{Create<AssetManager>()} {}

		virtual void Construct() override
		{
			Super::Construct();

			Log::Init("Saved/Logs");
			Log::Info("Initialize Context");
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

		static TPtr<Context> Get()
		{
			assert(globalInstance && "Context is not initialized! Call Context::Initialize().");
			return globalInstance;
		}
	};
}	 // namespace Rift
