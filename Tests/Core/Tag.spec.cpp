// Copyright 2015-2023 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/Core/Tag.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


go_bandit([]() {
	describe("Core.Tag", []() {
		it("Can copy empty", [&]() {
			Tag tag{};
			Tag tag2{};
			AssertThat(p::GetHash(tag)), Equals(0));
			AssertThat(tag.IsNone(), Equals(true));
			AssertThat(p::GetHash(tag2), Equals(0));
			AssertThat(tag2.IsNone(), Equals(true));
		});
		it("Can assign from literal", [&]() {
			Tag tag{"Kiwi"};
			AssertThat(tag.AsString(), Equals("Kiwi"));
		});

		it("Can assign from string", [&]() {
			String str{"Kiwi"};
			Tag tag{str};
			AssertThat(tag.AsString(), Equals("Kiwi"));
		});

		it("Can retrieve string data", [&]() {
			Tag tag{"Kiwi"};
			AssertThat(tag.AsString(), Equals("Kiwi"));
		});

		it("Can compare tags", [&]() {
			Tag tagKiwi{"Kiwi"};
			Tag tagKiwi2{"Kiwi"};
			Tag tagApple{"Apple"};
			AssertThat(tagKiwi, Equals(tagKiwi2));
			AssertThat(tagKiwi, !Equals(tagApple));
		});

		it("Different instances share string allocation", [&]() {
			Tag tagKiwi{"Kiwi"};
			Tag tagKiwi2{"Kiwi"};
			Tag tagApple{"Apple"};
			AssertThat(tagKiwi.AsString().data(), Equals(tagKiwi2.AsString().data()));
			AssertThat(tagKiwi.AsString().data(), !Equals(tagApple.AsString().data()));
		});

		it("Can check invalid/none", [&]() {
			Tag tagValid{"Kiwi"};
			Tag tagInvalid{};
			AssertThat(tagValid.IsNone(), Equals(false));
			AssertThat(tagValid, !Equals(Tag::None()));
			AssertThat(tagInvalid.IsNone(), Equals(true));
			AssertThat(tagInvalid, Equals(Tag::None()));
		});

		it("Contains correct hashes", [&]() {
			Tag tagKiwi{"Kiwi"};
			Tag tagKiwi2{"Kiwi"};
			AssertThat(p::GetHash(tagKiwi), Equals(p::GetHash(tagKiwi2)));
			AssertThat(p::GetHash(tagKiwi), Equals(p::GetHash("Kiwi")));
		});

		it("Can copy tag", [&]() {
			Tag tagKiwi{"Kiwi"};
			Tag tagApple{"Apple"};
			Tag tagCopy = tagKiwi;
			AssertThat(tagCopy.AsString(), Equals("Kiwi"));
			AssertThat(tagCopy, Equals(tagKiwi));
			AssertThat(tagCopy, !Equals(tagApple));
			tagCopy = tagApple;
			AssertThat(tagCopy.AsString(), Equals("Apple"));
			AssertThat(tagCopy, !Equals(tagKiwi));
			AssertThat(tagCopy, Equals(tagApple));
		});

		it("Can move tag", [&]() {
			Tag tagKiwi{"Kiwi"};
			Tag tagApple{"Apple"};
			Tag tagMove = Move(tagKiwi);
			AssertThat(tagKiwi, Equals(Tag::None()));
			AssertThat(tagMove.AsString(), Equals("Kiwi"));
			tagMove = Move(tagApple);
			AssertThat(tagApple, Equals(Tag::None()));
			AssertThat(tagMove.AsString(), Equals("Apple"));
		});
	});
});
