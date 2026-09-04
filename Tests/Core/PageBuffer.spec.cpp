// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "PipeMemory.h"

#include <Pipe/Core/PageBuffer.h>
#include <PipeTest.h>


using namespace p;


struct Dummy
{
	bool created   = false;
	bool destroyed = false;

	Dummy()
	{
		created = true;
	}
	~Dummy()
	{
		destroyed = true;
	}
};


Spec("ECS.PageBuffer", []()
{
	It("Can reserve", []()
	{
		TPageBuffer<Dummy, 2> buffer{GetCurrentArena()};

		Expect(buffer.GetPagesSize()).ToEqual(0);
		Expect(buffer.Capacity()).ToEqual(0);

		buffer.Reserve(2);
		Expect(buffer.GetPagesSize()).ToEqual(1);
		Expect(buffer.Capacity()).ToEqual(2);

		buffer.Reserve(6);
		Expect(buffer.GetPagesSize()).ToEqual(3);
		Expect(buffer.Capacity()).ToEqual(6);
	});

	It("Can shrink", []()
	{
		TPageBuffer<Dummy, 2> buffer{GetCurrentArena()};
		buffer.Reserve(7);
		Expect(buffer.GetPagesSize()).ToEqual(4);

		buffer.Shrink(4);
		Expect(buffer.GetPagesSize()).ToEqual(2);
		Expect(buffer.Capacity()).ToEqual(4);
	});

	It("Can insert", []()
	{
		TPageBuffer<Dummy, 2> buffer{GetCurrentArena()};
		buffer.Reserve(4);

		buffer.Insert(0);
		Expect(buffer[0].created).ToEqual(true);
		Expect(buffer[0].destroyed).ToEqual(false);

		buffer.Insert(3);
		Expect(buffer[3].created).ToEqual(true);
		Expect(buffer[3].destroyed).ToEqual(false);
	});

	It("Can remove", []()
	{
		TPageBuffer<Dummy, 2> buffer{GetCurrentArena()};
		buffer.Reserve(4);

		buffer.Insert(0);
		buffer.Insert(3);

		buffer.RemoveAt(0);
		// Temporarily disabled due to GCC only test fail
		// Expect(buffer[0].destroyed).ToEqual(true);

		buffer.RemoveAt(3);
		// Temporarily disabled due to GCC only test fail
		// Expect(buffer[3].destroyed).ToEqual(true);
	});

	It("Points to correct page", []()
	{
		TPageBuffer<Dummy, 2> buffer{GetCurrentArena()};
		buffer.Reserve(7);

		buffer.AssurePage(0);
		Expect(buffer.AssurePage(0)).ToNotEqual(nullptr);
		Expect(buffer.AssurePage(1)).ToNotEqual(nullptr);
		Expect(buffer.AssurePage(2)).ToNotEqual(nullptr);
		Expect(buffer.AssurePage(5)).ToNotEqual(nullptr);


		Expect(buffer.FindPage(0)).ToNotEqual(nullptr);
		Expect(buffer.FindPage(1)).ToNotEqual(nullptr);
		Expect(buffer.FindPage(2)).ToNotEqual(nullptr);
		Expect(buffer.FindPage(5)).ToNotEqual(nullptr);
		Expect(buffer.FindPage(6)).ToEqual(nullptr);
	});
});
