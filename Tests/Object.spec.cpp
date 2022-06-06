// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <PRefl/Class.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


class TestClass : public Class
{
	CLASS(TestClass, Class);

public:
	bool bConstructed = false;

	virtual void Construct() override
	{
		Super::Construct();
		bConstructed = true;
	}
};


go_bandit([]() {
	describe("Class", []() {
		describe("Pointers", []() {
			it("Can create object", [&]() {
				auto owner = MakeOwned<TestClass>();

				AssertThat(owner.Get(), Is().Not().EqualTo(nullptr));
				AssertThat(owner->bConstructed, Equals(true));
			});

			it("Can create object with owner", [&]() {
				auto owner  = MakeOwned<TestClass>();
				auto owner2 = MakeOwned<TestClass>(owner);

				AssertThat(owner2->bConstructed, Equals(true));
				AssertThat(owner2->GetOwner().IsValid(), Equals(true));
			});
		});
	});
});
