// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Pipe/Core/Log.h"
#include "Pipe/Core/Subprocess.h"

#include <Pipe/Files/PlatformPaths.h>
#include <PipeTest.h>


using namespace p;


P_SPEC("Core.Subprocess", []()
{
	It("Can run process", []()
	{
		Expect(p::RunProcess({""}).IsSet()).ToEqual(false);

#if defined(P_PLATFORM_WINDOWS)
		Expect(p::RunProcess({"cmd", "/c", "exit", "0"}).IsSet()).ToEqual(true);
#endif
	});
});
