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
		});
	});
});
