// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreObject.h"

#include "UI/Window.h"

#include "Assets/BaseAsset.h"
#include "MemberVariablesWindow.h"
#include "MemberFunctionsWindow.h"
#include "LocalVariablesWindow.h"
#include "FunctionGraphWindow.h"


class CodeEditor : public Window
{
	CLASS(CodeEditor, Window)

	Ptr<BaseAsset> asset;

	GlobalPtr<MemberVariablesWindow> variables;
	GlobalPtr<MemberFunctionsWindow> functions;
	GlobalPtr<LocalVariablesWindow> localVariables;
	GlobalPtr<FunctionGraphWindow> functionGraph;

	ImGuiWindowClass codeDockClass;
	ImGuiID codeDock;

public:

	virtual void Build() override;

	virtual void Tick(float deltaTime) override;
	virtual void TickContent(float deltaTime) override;

	virtual void ExpandViewsMenu() {}
};

