// Copyright 2015-2021 Piperift - All rights reserved
#include "Assets/AssetInfo.h"

#include "Serialization/Archive.h"


#if WITH_EDITOR
#	include "UI/Widgets/Properties/AssetInfoPropertyWidget.h"
#endif


namespace Rift
{
	bool AssetInfo::Serialize(class Archive& ar, const char* name)
	{
		ar(name, id);
		return true;
	}

#if WITH_EDITOR
	Class* AssetInfo::GetDetailsWidgetClass()
	{
		return AssetInfoPropertyWidget::Type();
	}
#endif
}	 // namespace Rift
