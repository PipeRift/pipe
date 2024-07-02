// Copyright 2015-2024 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/Core/String.h>
#include <Pipe/Core/StringView.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


go_bandit([]() {
	describe("Core.String", []() {
		it("Can assign from literal", [&]() {
			String v{"Kiwi"};
			AssertThat(v, Equals("Kiwi"));
			AssertThat(v.size(), Equals(4));
		});

		it("Can assign from string view", [&]() {
			StringView str{"Kiwi"};
			String v{str};
			AssertThat(v, Equals("Kiwi"));
			AssertThat(v.size(), Equals(4));
		});

		it("Can copy empty", [&]() {
			String str{};
			String str2{" "};
			AssertThat(str.empty(), Equals(true));
			AssertThat(str2.empty(), Equals(false));
			str2 = str;
			AssertThat(str2.empty(), Equals(true));
		});

		it("Can retrieve string data", [&]() {
			String v{"Kiwi"};
			AssertThat(v.data(), Equals("Kiwi"));
			AssertThat(v.size(), Equals(4));
			String v2{};
			AssertThat(v2.size(), Equals(0));
		});

		it("Can compare", [&]() {
			String vKiwi{"Kiwi"};
			String vKiwi2{"Kiwi"};
			String vApple{"Apple"};
			AssertThat(vKiwi, Equals(vKiwi2));
			AssertThat(vKiwi, !Equals(vApple));
		});

		it("Can copy", [&]() {
			String vKiwi{"Kiwi"};
			String vApple{"Apple"};
			String vCopy = vKiwi;
			AssertThat(vCopy, Equals("Kiwi"));
			AssertThat(vCopy, Equals(vKiwi));
			AssertThat(vCopy, !Equals(vApple));
			vCopy = vApple;
			AssertThat(vCopy, Equals("Apple"));
			AssertThat(vCopy, !Equals(vKiwi));
			AssertThat(vCopy, Equals(vApple));
		});

		it("Can move", [&]() {
			String vKiwi{"Kiwi"};
			String vApple{"Apple"};
			String vMove = Move(vKiwi);
			AssertThat(vKiwi.size(), Equals(0));
			AssertThat(vMove, Equals("Kiwi"));
			vMove = Move(vApple);
			AssertThat(vApple.size(), Equals(0));
			AssertThat(vMove, Equals("Apple"));
		});

		describe("Strings", []() {
			it("ToSentenceCase", [&]() {
				AssertThat(Strings::ToSentenceCase(""), Equals(""));
				AssertThat(Strings::ToSentenceCase("papa"), Equals("Papa"));
				AssertThat(Strings::ToSentenceCase("papa "), Equals("Papa "));
				AssertThat(Strings::ToSentenceCase("papa3"), Equals("Papa 3"));
				AssertThat(Strings::ToSentenceCase("MisterPotato"), Equals("Mister Potato"));
			});

			it("Convert u16 to u8", [&]() {
				TString<Char16> utf16string{0x41, 0x0448, 0x65e5, 0xd834, 0xdd1e};
				TString<Char8> u = Strings::Convert<TString<Char8>>(utf16string);
				AssertThat(u.size(), Equals(10));
			});
			it("Convert u8 to u16", [&]() {
				TString<AnsiChar> utf8_with_surrogates = "\xe6\x97\xa5\xd1\x88\xf0\x9d\x84\x9e";
				TString<Char16> utf16result =
				    Strings::Convert<TString<Char16>>(utf8_with_surrogates);
				AssertThat(utf16result.size(), Equals(4));
				AssertThat(utf16result[2] == 0xd834, Is().True());
				AssertThat(utf16result[3] == 0xdd1e, Is().True());
			});
			it("Convert u32 to u8", [&]() {
				TString<Char32> utf32string = {0x448, 0x65E5, 0x10346};
				TString<Char8> utf8result   = Strings::Convert<TString<Char8>>(utf32string);
				AssertThat(utf8result.size(), Equals(9));
			});
			it("Convert u8 to u32", [&]() {
				TString<AnsiChar> twochars  = "\xe6\x97\xa5\xd1\x88";
				TString<Char32> utf32result = Strings::Convert<TString<Char32>>(twochars);
				AssertThat(utf32result.size(), Equals(2));
			});
		});
	});
});
