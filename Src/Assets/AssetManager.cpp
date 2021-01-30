// Copyright 2015-2021 Piperift - All rights reserved

#include "Assets/AssetManager.h"
#include "Context.h"
#include "Files/FileSystem.h"
#include "Profiler.h"
#include "Tasks.h"


namespace Rift
{
	struct FAssetLoadingData
	{
		Refl::Class* type = nullptr;
		Json json;
	};


	Ptr<AssetData> AssetManager::Load(AssetInfo info)
	{
		const auto loaded = Load(TArray<AssetInfo>{MoveTemp(info)});
		if (loaded.Size() > 0)
		{
			return loaded.First();
		}
		return {};
	}

	TArray<Ptr<AssetData>> AssetManager::Load(TArray<AssetInfo> infos)
	{
		if (infos.Size() <= 0)
		{
			return {};
		}

		ZoneScopedC(0x459bd1);

		TArray<Ptr<AssetData>> finalAssets;
		{
			ZoneScopedNC("Ignore already loaded assets", 0xD19D45);
			for (i32 I = 0; I < infos.Size(); ++I)
			{
				if (Ptr<AssetData> loadedAsset = GetLoadedAsset(infos[I]))
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

			if (!FileSystem::LoadJsonFile(info.GetStrPath(), data.json))
			{
				Log::Error("Asset ({}) could not be loaded from disk", info.GetStrPath());
				return;
			}

			const auto type = data.json["asset_type"];
			if (!type.is_string())
			{
				Log::Error("Asset ({}) must have a type (asset_type)", info.GetStrPath());
				return;	   // Asset doesn't have a type
			}

			// Get asset type from json
			const String typeStr = type.get<String>();
			data.type = AssetData::StaticType()->FindChild(typeStr);
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
			auto newAsset = data.type->CreateInstance(Self()).Cast<AssetData>();
			const auto& info = infos[I];

			if (newAsset->OnLoad(info, data.json))
			{
				// Loading succeeded, registry the asset
				finalAssets.Add(newAsset);
				loadedAssets[info] = MoveTemp(newAsset);

				Log::Info("Loaded asset '{}'", info.GetStrPath());
			}
		}
		return finalAssets;
	}

	Ptr<AssetData> AssetManager::LoadOrCreate(const AssetInfo& info, Refl::Class* assetType)
	{
		if (info.IsNull() || !FileSystem::IsFolder(info.GetStrPath()))
		{
			return {};
		}

		Ptr<AssetData> loadedAsset = Load(info);
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
				const Ptr<AssetData> newAssetPtr = newAsset;

				loadedAssets[info.GetPath()] = MoveTemp(newAsset);

				return newAssetPtr;
			}
		}
		return {};
	}

	Ptr<AssetManager> AssetManager::Get()
	{
		return Context::Get()->GetAssetManager();
	}
}	 // namespace Rift
