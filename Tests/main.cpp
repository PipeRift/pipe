// Copyright 2015-2022 Piperift - All rights reserved

#include <Pipe/Memory/NewDelete.h>
PIPE_OVERRIDE_NEW_DELETE

#include <bandit/bandit.h>
#include <Pipe/Core/Backward.h>
#include <Pipe/Core/Log.h>


namespace backward
{
	backward::SignalHandling sh;
}    // namespace backward


int main(int argc, char* argv[])
{
	p::Log::Init({});    // No folder logging
	int result = bandit::run(argc, argv);
	p::Log::Shutdown();
	return result;
}
