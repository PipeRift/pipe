// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Pipe/Core/Log.h"
#include "Pipe/Core/Subprocess.h"

#include <PipeTest.h>
#include <Pipe/Files/PlatformPaths.h>


using namespace p;


Spec("Core.Subprocess", []()
{
It("Can run process", []()
{
	Expect(p::RunProcess({""}).IsSet()).ToEqual(false);

#if defined(_MSC_VER)    // Test with a silent command (no stdout)
	Expect(p::RunProcess({"cmd", "/c", "exit", "0"}).IsSet()).ToEqual(true);
#endif
});
});
