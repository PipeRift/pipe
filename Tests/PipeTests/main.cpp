// Copyright 2015-2026 Piperift. All Rights Reserved.

// NOTE: PipeNewDelete is deliberately not included here. PipeTestsLib provides the
// replacement operator new/delete (P_OVERRIDE_NEWDELETE) in its own translation unit;
// including it here too would cause duplicate-definition linker errors.

#include <Pipe.h>
#include <PipeTests.h>


void RegisterPipeTests();


int main(int argc, char* argv[])
{
	p::Initialize();
	RegisterPipeTests();
	int result = p::RunTests(argc, argv);
	p::Shutdown();
	return result;
}
