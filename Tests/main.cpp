// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Context.h>


class NoFileLogContext : public Pipe::Context
{
public:
	NoFileLogContext() : Pipe::Context(Pipe::Path{}) {}
};


int main(int argc, char* argv[])
{
	Pipe::InitializeContext<NoFileLogContext>();
	int result = bandit::run(argc, argv);
	Pipe::ShutdownContext();
	return result;
}
