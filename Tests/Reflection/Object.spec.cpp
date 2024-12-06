// Copyright 2015-2024 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <PipeReflect.h>


using namespace snowhouse;
using namespace bandit;


class TestObject : public p::Object
{
public:
	using Super = p::Object;
	P_CLASS(TestObject);

	bool bConstructed = false;

	TestObject()
	{
		bConstructed = true;
	}
};


go_bandit([]() {
	describe("Reflection.Object", []() {
		describe("Pointers", []() {
			it("Can create object", [&]() {
				auto owner = p::MakeOwned<TestObject>();

				AssertThat(owner.Get(), Is().Not().EqualTo(nullptr));
				AssertThat(owner->bConstructed, Equals(true));
			});

			it("Can create object with owner", [&]() {
				auto owner  = p::MakeOwned<TestObject>();
				auto owner2 = p::MakeOwned<TestObject>(owner);

				AssertThat(owner2->bConstructed, Equals(true));
				AssertThat(owner2->GetOwner().IsValid(), Equals(true));
			});
		});
	});
});
