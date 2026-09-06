// Copyright 2015-2026 Piperift. All Rights Reserved.

// NOTE: PipeNewDelete is deliberately not included here. PipeTest provides the
// replacement operator new/delete (P_OVERRIDE_NEWDELETE) in its own translation unit;
// including it here too would cause duplicate-definition linker errors.

#include <Pipe.h>
#include <PipeMemoryArenas.h>
#include <PipeTest.h>

#include <cstdlib>


// namespace backward
//{
//	backward::SignalHandling sh;
// }    // namespace backward


int main(int argc, char* argv[])
{
	p::Initialize();
	// Suppress leak messages from the global HeapArena (used internally by
	// many subsystems; not all of them free every allocation during tests).
	p::GetHeapArena().GetStats()->detectLeaks = false;

	// Specs auto-register at file scope via static init; just run them.
	int result = p::RunTests(argc, argv);
	p::Shutdown();
	return result;
}
