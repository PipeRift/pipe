// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Core/StringView.h"

#include <functional>


namespace p
{
	void Spec(StringView name, std::function<void()> fn);
	void Spec(std::function<void()> fn);
	void Describe(StringView name, std::function<void()> fn);
	void It(StringView name, std::function<void()> fn);
	void XIt(StringView name, std::function<void()> fn);
	void BeforeEach(std::function<void()> fn);
	void AfterEach(std::function<void()> fn);
	int RunTests(int argc, char** argv);
};    // namespace p
