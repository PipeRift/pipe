// Copyright 2015-2021 Piperift - All rights reserved

#include "Assets/AssetData.h"

#include "Files/Files.h"
#include "Serialization/Contexts.h"
#include "Serialization/Formats/JsonFormat.h"


namespace Rift
{
	bool AssetData::OnCreate(const AssetInfo& inInfo)
	{
		info = inInfo;

		return PostLoad(true);
	}

	bool AssetData::OnLoad(const AssetInfo& inInfo, String& data)
	{
		info = inInfo;

		// Deserialize asset
		Serl::JsonFormatReader reader{data};
		reader.GetContext().Serialize(*this);

		return PostLoad(false);
	}

	bool AssetData::SaveToPath(const Name& path)
	{
		const String& sPath = path.ToString();
		if (sPath.empty() || Files::IsFolder(sPath))
			return false;

		Serl::JsonFormatWriter writer;
		Serl::WriteContext& ct = writer;
		ct.BeginObject();
		ct.Next("asset_type", GetClass()->GetName());
		ct.Serialize(*this);

		return Files::SaveStringFile(sPath, String(writer.ToString()));
	}

	bool AssetData::Save()
	{
		return SaveToPath(info.GetPath());
	}
}    // namespace Rift
