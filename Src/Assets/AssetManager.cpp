// Copyright 2015-2021 Piperift - All rights reserved

#include "Assets/AssetManager.h"

#include "Context.h"
#include "Files/Files.h"
#include "Profiler.h"
#include "Tasks.h"


namespace Rift
{
	struct FAssetLoadingData
	{
		Refl::ClassType* type = nullptr;
		Json json;
	};


	TPtr<AssetData> AssetManager::Load(AssetInfo info)
	{
		const auto loaded = Load(TArray<AssetInfo>{Move(info)});
		if (loaded.Size() > 0)
		{
			return loaded.First();
		}
		return {};
	}

	TArray<TPtr<AssetData>> AssetManager::Load(TArray<AssetInfo> infos)
	{
		if (infos.Size() <= 0)
		{
			return {};
		}

		ZoneScopedC(0x459bd1);

		TArray<TPtr<AssetData>> finalAssets;
		{
			ZoneScopedNC("Ignore already loaded assets", 0xD19D45);
			for (i32 I = 0; I < infos.Size(); ++I)
			{
				if (TPtr<AssetData> loadedAsset = GetLoadedAsset(infos[I]))
				{
					infos.RemoveAtSwap(I, false);
					finalAssets.Add(loadedAsset);
				}
			}
		}

		TaskFlow loadTask;
		TArray<FAssetLoadingData> loadedDatas(infos.Size());

		loadTask.for_each_index(0, infos.Size(), 1, [&loadedDatas, &infos](i32 i) {
			ZoneScopedNC("Load Asset File", 0xD19D45);
			auto& info = infos[i];
			auto& data = loadedDatas[i];

			if (!Files::LoadJsonFile(info.GetStrPath(), data.json))
			{
				Log::Error("Asset ({}) could not be loaded from disk", info.GetStrPath());
				return;
			}

			const auto type = data.json["asset_type"];
			if (!type.is_string())
			{
				Log::Error("Asset ({}) must have a type (asset_type)", info.GetStrPath());
				return;    // Asset doesn't have a type
			}

			// Get asset type from json
			const String typeStr = type.get<String>();

			data.type = GetType<AssetData>()->FindChild(typeStr);
			if (!data.type)
			{
				Log::Error("Asset ({}) has unknown asset_type '{}' ", info.GetStrPath(), typeStr);
			}
		});
		TaskSystem::Get().RunFlow(loadTask).wait();

		// Deserialize asset instances
		for (i32 I = 0; I < infos.Size(); ++I)
		{
			auto& data = loadedDatas[I];
			if (!data.type)
			{
				continue;
			}

			ZoneScopedNC("Deserialize asset", 0xD19D45);

			// Create the asset instance
			auto newAsset    = data.type->CreateInstance(Self()).Cast<AssetData>();
			const auto& info = infos[I];

			if (newAsset->OnLoad(info, data.json))
			{
				// Loading succeeded, registry the asset
				finalAssets.Add(newAsset);
				loadedAssets[info] = Move(newAsset);

				Log::Info("Loaded asset '{}'", info.GetStrPath());
			}
		}
		return finalAssets;
	}

	TPtr<AssetData> AssetManager::LoadOrCreate(const AssetInfo& info, Refl::ClassType* assetType)
	{
		if (info.IsNull() || !Files::IsFolder(info.GetStrPath()))
		{
			return {};
		}

		TPtr<AssetData> loadedAsset = Load(info);
		if (loadedAsset)
		{
			return loadedAsset;
		}

		if (!assetType)
			return {};

		if (assetType->IsChildOf<AssetData>())
		{
			auto newAsset = Create<AssetData>(assetType, Self());
			if (newAsset->OnCreate(info))
			{
				const TPtr<AssetData> newAssetPtr = newAsset;

				loadedAssets[info.GetPath()] = Move(newAsset);

				return newAssetPtr;
			}
		}
		return {};
	}

	TPtr<AssetManager> AssetManager::Get()
	{
		return Context::Get()->GetAssetManager();
	}
}    // namespace Rift
