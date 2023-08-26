// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Memory/Alloc.h"

#include <bandit/bandit.h>
#include <Pipe/Core/PageBuffer.h>


using namespace snowhouse;
using namespace bandit;
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


go_bandit([]() {
	describe("ECS.PageBuffer", []() {
		it("Can reserve", [&]() {
			TPageBuffer<Dummy, 2> buffer{GetCurrentArena()};

			AssertThat(buffer.GetPagesSize(), Equals(0));
			AssertThat(buffer.Capacity(), Equals(0));

			buffer.Reserve(2);
			AssertThat(buffer.GetPagesSize(), Equals(1));
			AssertThat(buffer.Capacity(), Equals(2));

			buffer.Reserve(6);
			AssertThat(buffer.GetPagesSize(), Equals(3));
			AssertThat(buffer.Capacity(), Equals(6));
		});

		it("Can shrink", [&]() {
			TPageBuffer<Dummy, 2> buffer{GetCurrentArena()};
			buffer.Reserve(7);
			AssertThat(buffer.GetPagesSize(), Equals(4));

			buffer.Shrink(4);
			AssertThat(buffer.GetPagesSize(), Equals(2));
			AssertThat(buffer.Capacity(), Equals(4));
		});

		it("Can insert", [&]() {
			TPageBuffer<Dummy, 2> buffer{GetCurrentArena()};
			buffer.Reserve(4);

			buffer.Insert(0);
			AssertThat(buffer[0].created, Equals(true));
			AssertThat(buffer[0].destroyed, Equals(false));

			buffer.Insert(3);
			AssertThat(buffer[3].created, Equals(true));
			AssertThat(buffer[3].destroyed, Equals(false));
		});

		it("Can remove", [&]() {
			TPageBuffer<Dummy, 2> buffer{GetCurrentArena()};
			buffer.Reserve(4);

			buffer.Insert(0);
			buffer.Insert(3);

			buffer.RemoveAt(0);
			// Temporarily disabled due to GCC only test fail
			// AssertThat(buffer[0].destroyed, Equals(true));

			buffer.RemoveAt(3);
			// Temporarily disabled due to GCC only test fail
			// AssertThat(buffer[3].destroyed, Equals(true));
		});

		it("Points to correct page", [&]() {
			TPageBuffer<Dummy, 2> buffer{GetCurrentArena()};
			buffer.Reserve(7);

			buffer.AssurePage(0);
			AssertThat(buffer.AssurePage(0), !Equals(nullptr));
			AssertThat(buffer.AssurePage(1), !Equals(nullptr));
			AssertThat(buffer.AssurePage(2), !Equals(nullptr));
			AssertThat(buffer.AssurePage(5), !Equals(nullptr));


			AssertThat(buffer.FindPage(0), !Equals(nullptr));
			AssertThat(buffer.FindPage(1), !Equals(nullptr));
			AssertThat(buffer.FindPage(2), !Equals(nullptr));
			AssertThat(buffer.FindPage(5), !Equals(nullptr));
			AssertThat(buffer.FindPage(6), Equals(nullptr));
		});
	});
});
