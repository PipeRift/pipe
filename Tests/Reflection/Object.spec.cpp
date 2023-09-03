// Copyright 2015-2023 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/Reflect/Class.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


class TestClass : public Class
{
	P_CLASS(TestClass, Class);

public:
	bool bConstructed = false;

	TestClass()
	{
		bConstructed = true;
	}
};


go_bandit([]() {
	describe("Reflection.Class", []() {
		describe("Pointers", []() {
			it("Can create class", [&]() {
				auto owner = MakeOwned<TestClass>();

				AssertThat(owner.Get(), Is().Not().EqualTo(nullptr));
				AssertThat(owner->bConstructed, Equals(true));
			});

			it("Can create class with owner", [&]() {
				auto owner  = MakeOwned<TestClass>();
				auto owner2 = MakeOwned<TestClass>(owner);

				AssertThat(owner2->bConstructed, Equals(true));
				AssertThat(owner2->GetOwner().IsValid(), Equals(true));
			});
		});
	});
});
