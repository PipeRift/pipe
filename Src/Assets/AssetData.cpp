// Copyright 2015-2021 Piperift - All rights reserved
#include "Assets/AssetData.h"

#include "Files/Files.h"


namespace Rift
{
	bool AssetData::OnCreate(const AssetInfo& inInfo)
	{
		info = inInfo;
		return PostLoad();
	}

	bool AssetData::OnLoad(const AssetInfo& inInfo, Json& data)
	{
		info = inInfo;

		// Deserialize asset
		JsonArchive ar(data);
		Serialize(ar);

		return PostLoad();
	}

	bool AssetData::SaveToPath(const Name& path)
	{
		const String& sPath = path.ToString();
		if (sPath.empty() || !Files::IsFolder(sPath))
			return false;

		JsonArchive ar{};
		Name className = GetClass()->GetName();
		ar("asset_type", className);
		Serialize(ar);

		return Files::SaveJsonFile(sPath, ar.GetData(), ar.GetIndent());
	}

	bool AssetData::Save()
	{
		return SaveToPath(info.GetPath());
	}
}    // namespace Rift
