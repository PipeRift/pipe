// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeNewDelete.h>
//  Override as first include

#include <bandit/bandit.h>
#include <Pipe.h>


// namespace backward
//{
//	backward::SignalHandling sh;
// }    // namespace backward


int main(int argc, char* argv[])
{
	p::Initialize();
	int result = bandit::run(argc, argv);
	p::Shutdown();
	return result;
}
