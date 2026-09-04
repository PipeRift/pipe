// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeReflect.h>
#include <PipeTest.h>


using namespace p;


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


Spec("Reflection.Object", []()
{
	Describe("Pointers", []()
	{
		It("Can create object", []()
		{
			auto owner = p::MakeOwned<TestObject>();

			Expect(owner.Get()).ToNotEqual(nullptr);
			Expect(owner->bConstructed).ToEqual(true);
		});

		It("Can create object with owner", []()
		{
			auto owner  = p::MakeOwned<TestObject>();
			auto owner2 = p::MakeOwned<TestObject>(owner);

			Expect(owner2->bConstructed).ToEqual(true);
			Expect(owner2->GetOwner().Get()).ToEqual(owner.Get());
		});
	});
});
