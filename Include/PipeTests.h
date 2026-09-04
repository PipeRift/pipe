// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Core/StringView.h"

#include <functional>


namespace p
{
	/**
	 * Test framework for Pipe and Rift.
	 * Imgui-style global context: registration functions act on a current group.
	 * Spec opens a first-level group; Describe/It/BeforeEach/AfterEach attach to
	 * the current group as functions are called. Registration runs inside a
	 * function (e.g. a Register*Tests() routine called from main) — the framework
	 * uses no macros, so specs must not be registered at namespace scope.
	 */

	// Self-registering top-level. Spec(name, fn) opens a first group named `name`.
	void Spec(StringView name, std::function<void()> fn);
	// Nameless top-level (like go_bandit); use Describe inside fn.
	void Spec(std::function<void()> fn);

	// Nested group. Only valid inside a Spec; otherwise logs an error and ignores.
	void Describe(StringView name, std::function<void()> fn);
	// Register a runnable test in the current group.
	void It(StringView name, std::function<void()> fn);
	// Register a disabled test; never run.
	void XIt(StringView name, std::function<void()> fn);
	// Setup hook attached to the current group.
	void BeforeEach(std::function<void()> fn);
	// Teardown hook attached to the current group.
	void AfterEach(std::function<void()> fn);

	int RunTests(int argc, char** argv);
};    // namespace p
