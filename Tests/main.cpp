// Copyright 2015-2023 Piperift - All rights reserved

#include <Pipe/Memory/NewDelete.h>
P_OVERRIDE_NEW_DELETE

#include <bandit/bandit.h>
#include <Pipe/Core/Backward.h>
#include <Pipe/Pipe.h>


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
