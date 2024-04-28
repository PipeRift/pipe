// Copyright 2015-2024 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/Core/String.h>
#include <Pipe/Core/StringView.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


go_bandit([]() {
	describe("Core.StringView", []() {
		it("Can assign from literal", [&]() {
			StringView v{"Kiwi"};
			AssertThat(v, Equals("Kiwi"));
			AssertThat(v.size(), Equals(4));
		});

		it("Can assign from string", [&]() {
			String str{"Kiwi"};
			StringView v{str};
			AssertThat(v, Equals("Kiwi"));
			AssertThat(v.size(), Equals(4));
		});

		it("Can copy empty", [&]() {
			StringView str{};
			StringView str2{" "};
			AssertThat(str.empty(), Equals(true));
			AssertThat((u8*)str.data(), Equals(nullptr));
			AssertThat(str2.empty(), Equals(false));
			AssertThat((u8*)str2.data(), !Equals(nullptr));
			str2 = str;
			AssertThat(str2.empty(), Equals(true));
			AssertThat((u8*)str2.data(), Equals(nullptr));
		});

		it("Can retrieve string data", [&]() {
			StringView v{"Kiwi"};
			AssertThat(v.data(), Equals("Kiwi"));
			AssertThat(v.size(), Equals(4));
			StringView v2{};
			AssertThat((u8*)v2.data(), Equals(nullptr));
			AssertThat(v2.size(), Equals(0));
		});

		it("Can compare", [&]() {
			StringView vKiwi{"Kiwi"};
			StringView vKiwi2{"Kiwi"};
			StringView vApple{"Apple"};
			AssertThat(vKiwi, Equals(vKiwi2));
			AssertThat(vKiwi, !Equals(vApple));
		});

		it("Can copy", [&]() {
			StringView vKiwi{"Kiwi"};
			StringView vApple{"Apple"};
			StringView vCopy = vKiwi;
			AssertThat(vCopy, Equals("Kiwi"));
			AssertThat(vCopy, Equals(vKiwi));
			AssertThat(vCopy, !Equals(vApple));
			vCopy = vApple;
			AssertThat(vCopy, Equals("Apple"));
			AssertThat(vCopy, !Equals(vKiwi));
			AssertThat(vCopy, Equals(vApple));
		});

		it("Can move", [&]() {
			StringView vKiwi{"Kiwi"};
			StringView vApple{"Apple"};
			StringView vMove = Move(vKiwi);
			AssertThat(vMove, Equals("Kiwi"));
			vMove = Move(vApple);
			AssertThat(vMove, Equals("Apple"));
		});

		describe("Strings", []() {
			it("Can Find", [&]() {
				StringView v{"Kiwiwi"};

				// Find Chars
				AssertThat(Strings::Find(v, 'K', FindDirection::Front), Equals(0));
				AssertThat(Strings::Find(v, 'K', FindDirection::Back), Equals(0));
				AssertThat(Strings::Find(v, 'i', FindDirection::Front), Equals(1));
				AssertThat(Strings::Find(v, 'i', FindDirection::Back), Equals(5));

				// Find Sub-strings
				AssertThat(Strings::Find(v, "Ki", FindDirection::Front), Equals(0));
				AssertThat(Strings::Find(v, "Ki", FindDirection::Back), Equals(0));
				AssertThat(Strings::Find(v, "wi", FindDirection::Front), Equals(2));
				AssertThat(Strings::Find(v, "wi", FindDirection::Back), Equals(4));
			});
		});
	});
});
