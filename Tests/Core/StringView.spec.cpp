// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <Pipe/Core/StringView.h>
#include <PipeStrings.h>


using namespace p;


Spec("Strings", []()
{
Describe("StringView", []()
{
	It("Can assign from literal", []()
	{
		StringView v{"Kiwi"};
		Expect(v).ToEqual("Kiwi");
		Expect(v.size()).ToEqual(4);
	});

	It("Can assign from string", []()
	{
		String str{"Kiwi"};
		StringView v{str};
		Expect(v).ToEqual("Kiwi");
		Expect(v.size()).ToEqual(4);
	});

	It("Can copy empty", []()
	{
		StringView str{};
		StringView str2{" "};
		Expect(str.empty()).ToEqual(true);
		Expect((u8*)str.data()).ToEqual(nullptr);
		Expect(str2.empty()).ToEqual(false);
		Expect((u8*)str2.data()).ToNotEqual(nullptr);
		str2 = str;
		Expect(str2.empty()).ToEqual(true);
		Expect((u8*)str2.data()).ToEqual(nullptr);
	});

	It("Can retrieve string data", []()
	{
		StringView v{"Kiwi"};
		Expect(v.data()).ToEqual("Kiwi");
		Expect(v.size()).ToEqual(4);
		StringView v2{};
		Expect((u8*)v2.data()).ToEqual(nullptr);
		Expect(v2.size()).ToEqual(0);
	});

	It("Can compare", []()
	{
		StringView vKiwi{"Kiwi"};
		StringView vKiwi2{"Kiwi"};
		StringView vApple{"Apple"};
		Expect(vKiwi).ToEqual(vKiwi2);
		Expect(vKiwi).ToNotEqual(vApple);
	});

	It("Can copy", []()
	{
		StringView vKiwi{"Kiwi"};
		StringView vApple{"Apple"};
		StringView vCopy = vKiwi;
		Expect(vCopy).ToEqual("Kiwi");
		Expect(vCopy).ToEqual(vKiwi);
		Expect(vCopy).ToNotEqual(vApple);
		vCopy = vApple;
		Expect(vCopy).ToEqual("Apple");
		Expect(vCopy).ToNotEqual(vKiwi);
		Expect(vCopy).ToEqual(vApple);
	});

	It("Can move", []()
	{
		StringView vKiwi{"Kiwi"};
		StringView vApple{"Apple"};
		StringView vMove = Move(vKiwi);
		Expect(vMove).ToEqual("Kiwi");
		vMove = Move(vApple);
		Expect(vMove).ToEqual("Apple");
	});

	Describe("Strings", []()
	{
		It("Can Find", []()
		{
			StringView v{"Kiwiwi"};

			// Find Chars
			Expect(Strings::Find(v, 'K', FindDir::Front)).ToEqual(0);
			Expect(Strings::Find(v, 'K', FindDir::Back)).ToEqual(0);
			Expect(Strings::Find(v, 'i', FindDir::Front)).ToEqual(1);
			Expect(Strings::Find(v, 'i', FindDir::Back)).ToEqual(5);
			// Find last chars
			Expect(Strings::Find(v, 'w', FindDir::Front, true)).ToEqual(0);    // 'K'
			Expect(Strings::Find(v, 'w', FindDir::Back, true)).ToEqual(5);     // 'i'
			Expect(Strings::Find(v, 'K', FindDir::Front, true)).ToEqual(1);    // 'i'
			Expect(Strings::Find(v, 'i', FindDir::Back, true)).ToEqual(4);     // 'w'

			// Find Sub-strings
			Expect(Strings::Find(v, "Ki", FindDir::Front)).ToEqual(0);
			Expect(Strings::Find(v, "Ki", FindDir::Back)).ToEqual(0);
			Expect(Strings::Find(v, "wi", FindDir::Front)).ToEqual(2);
			Expect(Strings::Find(v, "wi", FindDir::Back)).ToEqual(4);
		});
	});
});
});
