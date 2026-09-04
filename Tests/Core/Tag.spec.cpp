// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTests.h>
#include <Pipe/Core/Tag.h>


using namespace p;


void RegisterCoreTagTests()
{
	Spec("Core.Tag", []()
	{
		It("Can copy empty", []()
		{
			Tag tag{};
			Tag tag2{"Ahh"};
			Expect(p::GetHash(tag)).ToEqual(0);
			Expect(tag.IsNone()).ToEqual(true);
			Expect(p::GetHash(tag2)).ToNotEqual(0);
			Expect(tag2.IsNone()).ToEqual(false);
			tag2 = tag;
			Expect(p::GetHash(tag2)).ToEqual(0);
			Expect(tag2.IsNone()).ToEqual(true);
		});
		It("Can assign from literal", []()
		{
			Tag tag{"Kiwi"};
			Expect(tag.AsString()).ToEqual("Kiwi");
		});

		It("Can assign from string", []()
		{
			String str{"Kiwi"};
			Tag tag{str};
			Expect(tag.AsString()).ToEqual("Kiwi");
		});

		It("Can retrieve string data", []()
		{
			Tag tag{"Kiwi"};
			Expect(tag.AsString()).ToEqual("Kiwi");
		});

		It("Can compare tags", []()
		{
			Tag tagKiwi{"Kiwi"};
			Tag tagKiwi2{"Kiwi"};
			Tag tagApple{"Apple"};
			Expect(tagKiwi).ToEqual(tagKiwi2);
			Expect(tagKiwi).ToNotEqual(tagApple);
		});

		It("Different instances share string allocation", []()
		{
			Tag tagKiwi{"Kiwi"};
			Tag tagKiwi2{"Kiwi"};
			Tag tagApple{"Apple"};
			Expect(tagKiwi.AsString().data()).ToEqual(tagKiwi2.AsString().data());
			Expect(tagKiwi.AsString().data()).ToNotEqual(tagApple.AsString().data());
		});

		It("Can check invalid/none", []()
		{
			Tag tagValid{"Kiwi"};
			Tag tagInvalid{};
			Expect(tagValid.IsNone()).ToEqual(false);
			Expect(tagValid).ToNotEqual(Tag::None());
			Expect(tagInvalid.IsNone()).ToEqual(true);
			Expect(tagInvalid).ToEqual(Tag::None());
		});

		It("Contains correct hashes", []()
		{
			Tag tagKiwi{"Kiwi"};
			Tag tagKiwi2{"Kiwi"};
			Expect(p::GetHash(tagKiwi)).ToEqual(p::GetHash(tagKiwi2));
			Expect(tagKiwi.GetStringHash()).ToEqual(p::GetHash("Kiwi"));
		});

		It("Can copy tag", []()
		{
			Tag tagKiwi{"Kiwi"};
			Tag tagApple{"Apple"};
			Tag tagCopy = tagKiwi;
			Expect(tagCopy.AsString()).ToEqual("Kiwi");
			Expect(tagCopy).ToEqual(tagKiwi);
			Expect(tagCopy).ToNotEqual(tagApple);
			tagCopy = tagApple;
			Expect(tagCopy.AsString()).ToEqual("Apple");
			Expect(tagCopy).ToNotEqual(tagKiwi);
			Expect(tagCopy).ToEqual(tagApple);
		});

		It("Can move tag", []()
		{
			Tag tagKiwi{"Kiwi"};
			Tag tagApple{"Apple"};
			Tag tagMove = Move(tagKiwi);
			Expect(tagKiwi).ToEqual(Tag::None());
			Expect(tagMove.AsString()).ToEqual("Kiwi");
			tagMove = Move(tagApple);
			Expect(tagApple).ToEqual(Tag::None());
			Expect(tagMove.AsString()).ToEqual("Apple");
		});
	});
}
