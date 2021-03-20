// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "AssetData.h"
#include "AssetInfo.h"
#include "Containers/Map.h"
#include "CoreObject.h"
#include "Files/FileSystem.h"


namespace Rift
{
	class AssetManager : public Object
	{
		CLASS(AssetManager, Object)

	public:
		static CORE_API constexpr StringView assetFormat{".rf"};

	private:
		TArray<AssetInfo> assetInfos;

		TMap<AssetInfo, ObjectPtr<AssetData>> loadedAssets{};


	public:
		CORE_API Ptr<AssetData> Load(AssetInfo info);
		CORE_API TArray<Ptr<AssetData>> Load(TArray<AssetInfo> infos);

		CORE_API Ptr<AssetData> LoadOrCreate(const AssetInfo& info, Refl::ClassType* assetType);

		CORE_API Ptr<AssetData> GetLoadedAsset(const AssetInfo& id) const
		{
			if (const auto* asset = loadedAssets.Find(id))
			{
				return asset->AsPtr();
			}
			return {};
		}

		CORE_API bool IsLoaded(const AssetInfo& id) const
		{
			return loadedAssets.Contains(id);
		}

		static CORE_API Ptr<AssetManager> Get();
	};
}    // namespace Rift
