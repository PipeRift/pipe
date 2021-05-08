// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "AssetData.h"
#include "AssetInfo.h"
#include "Containers/Map.h"
#include "CoreObject.h"
#include "Events/Broadcast.h"


namespace Rift
{
	class AssetManager : public Object
	{
		CLASS(AssetManager, Object)

	public:
		static CORE_API constexpr StringView assetFormat{".rf"};

	private:
		TArray<AssetInfo> assetInfos;

		TMap<AssetInfo, TOwnPtr<AssetData>> loadedAssets{};

		TBroadcast<AssetInfo, AssetInfo> onAssetRenamed;


	public:
		CORE_API TPtr<AssetData> Load(AssetInfo info);
		CORE_API TArray<TPtr<AssetData>> Load(TArray<AssetInfo> infos);

		CORE_API TPtr<AssetData> LoadOrCreate(const AssetInfo& info, Refl::ClassType* assetType);

		CORE_API TPtr<AssetData> GetLoadedAsset(const AssetInfo& id) const
		{
			if (const auto* asset = loadedAssets.Find(id))
			{
				return asset->AsPtr();
			}
			return {};
		}

		void Rename(AssetInfo asset, Name newName);

		// Broadcast notified when an asset is renamed (AssetInfo lastAsset, AssetInfo newAsset)
		CORE_API const TBroadcast<AssetInfo, AssetInfo>& OnAssetRenamed()
		{
			return onAssetRenamed;
		}

		CORE_API bool IsLoaded(const AssetInfo& id) const
		{
			return loadedAssets.Contains(id);
		}

		static CORE_API TPtr<AssetManager> Get();
	};
}    // namespace Rift
