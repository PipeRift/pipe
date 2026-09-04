// Copyright 2015-2026 Piperift. All Rights Reserved.

// NOTE: PipeNewDelete is deliberately not included here. PipeTest provides the
// replacement operator new/delete (P_OVERRIDE_NEWDELETE) in its own translation unit;
// including it here too would cause duplicate-definition linker errors.

#include <Pipe.h>
#include <PipeTest.h>


int main(int argc, char* argv[])
{
	p::Initialize();
	int result = p::RunTests(argc, argv);
	p::Shutdown();
	return result;
}
