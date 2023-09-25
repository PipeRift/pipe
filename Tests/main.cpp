// Copyright 2015-2023 Piperift - All rights reserved

#include <Pipe/Memory/NewDelete.h>
//  Override as first include

#include <bandit/bandit.h>
#include <Pipe.h>
#include <Pipe/Core/Backward.h>



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
