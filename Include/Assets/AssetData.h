// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "AssetInfo.h"
#include "CoreObject.h"


namespace Rift
{
	class AssetData : public Object
	{
		CLASS(AssetData, Object)

		AssetInfo info;


	public:
		AssetData() : Super() {}

		/** Called after an asset is created. Internal usage only. */
		bool OnCreate(const AssetInfo& inInfo);

		/** Deserializes the asset. Internal usage only. */
		bool OnLoad(const AssetInfo& inInfo, String& data);


		CORE_API bool SaveToPath(const Name& path);

		CORE_API bool Save();


	protected:
		/** Called after the asset was loaded or created */
		virtual bool PostLoad(bool justCreated)
		{
			return true;
		}
		void BeforeDestroy() override
		{
			OnUnload();
		}
		virtual void OnUnload() {}

		/** HELPERS */
	public:
		const AssetInfo& GetInfo() const
		{
			return info;
		}

		const Name& GetMetaPath() const
		{
			return info.GetPath();
		}
		const Name& GetRawPath() const
		{
			return info.GetPath();
		}
	};
}    // namespace Rift
