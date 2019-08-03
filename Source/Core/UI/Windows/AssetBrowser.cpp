// Copyright 2015-2019 Piperift - All rights reserved

#include "AssetBrowser.h"
#include "Core/Files/FileSystem.h"


#if WITH_EDITOR

void AssetBrowser::Build()
{
	Super::Build();
	bOpen = true;
	SetName(TX("Asset Browser"));
	windowFlags = ImGuiWindowFlags_AlwaysAutoResize;
}

void AssetBrowser::Tick(float)
{
}

#endif
