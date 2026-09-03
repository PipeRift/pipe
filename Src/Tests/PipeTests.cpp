// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "PipeTests.h"


namespace p
{
	void Spec(StringView name, std::function<void()>) { (void)name; }
	void Spec(std::function<void()>) {}
	void Describe(StringView name, std::function<void()>) { (void)name; }
	void It(StringView name, std::function<void()>) { (void)name; }
	void XIt(StringView name, std::function<void()>) { (void)name; }
	void BeforeEach(std::function<void()>) {}
	void AfterEach(std::function<void()>) {}
	int RunTests(int argc, char** argv) { (void)argc; (void)argv; return 0; }
};    // namespace p
