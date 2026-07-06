// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeNewDelete.h>
//  Override as first include

#include <bandit/bandit.h>
#include <Pipe.h>
#include <PipeMemoryArenas.h>


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
	int result = bandit::run(argc, argv);
	p::Shutdown();
	return result;
}
