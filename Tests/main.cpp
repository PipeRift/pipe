// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Context.h>


class NoFileLogContext : public pipe::Context
{
public:
	NoFileLogContext() : pipe::Context(pipe::Path{}) {}
};


int main(int argc, char* argv[])
{
	pipe::InitializeContext<NoFileLogContext>();
	int result = bandit::run(argc, argv);
	pipe::ShutdownContext();
	return result;
}
