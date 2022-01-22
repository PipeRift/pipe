// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Context.h>


class NoFileLogContext : public Rift::Context
{
public:
	NoFileLogContext() : Rift::Context(Rift::Path{}) {}
};


int main(int argc, char* argv[])
{
	Rift::InitializeContext<NoFileLogContext>();
	int result = bandit::run(argc, argv);
	Rift::ShutdownContext();
	return result;
}
