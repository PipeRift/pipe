// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <bandit/bandit.h>
#include <Pipe/Core/StringView.h>
#include <PipeMemoryArenas.h>
#include <PipeStrings.h>

#include <algorithm>
#include <format>


using namespace snowhouse;
using namespace bandit;
using namespace p;

// Longer than the inline capacity, forcing heap allocations
static const StringView longText = "0123456789ABCDEFGHIJ0123456789ABC";


go_bandit([]()
{
	describe("Strings.String", []()
	{
		describe("Construction", []()
		{
			it("Can default construct", [&]()
			{
				String v{};
				AssertThat(v.size(), Equals(0u));
				AssertThat(v.empty(), Is().True());
				AssertThat(v.length(), Equals(0u));
				// c_str() must always return a valid pointer to a null terminator
				AssertThat(v.c_str() != nullptr, Is().True());
				AssertThat(v.c_str()[0], Equals('\0'));
				AssertThat(v.data() != nullptr, Is().True());
				AssertThat(v.data()[0], Equals('\0'));
			});

			it("Can construct from literal", [&]()
			{
				String v{"Kiwi"};
				AssertThat(v, Equals("Kiwi"));
				AssertThat(v.size(), Equals(4u));
			});

			it("Can construct from literal with count", [&]()
			{
				String v{"KiwiApple", 4};
				AssertThat(v, Equals("Kiwi"));
				AssertThat(v.size(), Equals(4u));
			});

			it("Can construct from count and char", [&]()
			{
				String v(5, 'x');
				AssertThat(v, Equals("xxxxx"));
				AssertThat(v.size(), Equals(5u));
			});

			it("Can construct from string view", [&]()
			{
				StringView str{"Kiwi"};
				String v{str};
				AssertThat(v, Equals("Kiwi"));
				AssertThat(v.size(), Equals(4u));
			});

			it("Can construct from string view with pos and count", [&]()
			{
				StringView str{"KiwiApple"};
				String v{str, 4, 5};
				AssertThat(v, Equals("Apple"));
			});

			it("Can construct from substring", [&]()
			{
				String str{"KiwiApple"};
				String v{str, 4};
				AssertThat(v, Equals("Apple"));
				String v2{str, 4, 3};
				AssertThat(v2, Equals("App"));
			});

			it("Can construct from iterators", [&]()
			{
				std::string_view sv = "Kiwi";
				String v{sv.begin(), sv.end()};
				AssertThat(v, Equals("Kiwi"));
			});

			it("Can construct from initializer list", [&]()
			{
				String v{'K', 'i', 'w', 'i'};
				AssertThat(v, Equals("Kiwi"));
			});

			it("Can copy construct", [&]()
			{
				String v{"Kiwi"};
				String v2{v};
				AssertThat(v2, Equals("Kiwi"));
				AssertThat(v, Equals("Kiwi"));
			});

			it("Can move construct", [&]()
			{
				String v{"Kiwi"};
				String v2{Move(v)};
				AssertThat(v2, Equals("Kiwi"));
				// Moved-from string is valid and empty
				AssertThat(v.size(), Equals(0u));
				AssertThat(v.empty(), Is().True());
				AssertThat(v.c_str()[0], Equals('\0'));
			});
		});

		describe("Assignment", []()
		{
			it("Can assign from literal", [&]()
			{
				String v;
				v = "Kiwi";
				AssertThat(v, Equals("Kiwi"));
			});

			it("Can copy assign", [&]()
			{
				String vKiwi{"Kiwi"};
				String vApple{"Apple"};
				String vCopy = vKiwi;
				AssertThat(vCopy, Equals("Kiwi"));
				vCopy = vApple;
				AssertThat(vCopy, Equals("Apple"));
				AssertThat(vCopy, Equals(vApple));
			});

			it("Can move assign", [&]()
			{
				String vKiwi{"Kiwi"};
				String vApple{"Apple"};
				String vMove = Move(vKiwi);
				AssertThat(vKiwi.size(), Equals(0u));
				AssertThat(vMove, Equals("Kiwi"));
				vMove = Move(vApple);
				AssertThat(vApple.size(), Equals(0u));
				AssertThat(vMove, Equals("Apple"));
			});

			it("Can assign char", [&]()
			{
				String v;
				v = 'x';
				AssertThat(v, Equals("x"));
			});

			it("Can assign initializer list", [&]()
			{
				String v;
				v = {'K', 'i', 'w', 'i'};
				AssertThat(v, Equals("Kiwi"));
			});

			it("Can assign string view", [&]()
			{
				String v;
				StringView sv{"Kiwi"};
				v = sv;
				AssertThat(v, Equals("Kiwi"));
			});

			it("Can assign", [&]()
			{
				String v;
				v.assign("Kiwi");
				AssertThat(v, Equals("Kiwi"));
				v.assign("KiwiApple", 4);
				AssertThat(v, Equals("Kiwi"));
				v.assign(3, 'x');
				AssertThat(v, Equals("xxx"));
				String other{"Apple"};
				v.assign(other);
				AssertThat(v, Equals("Apple"));
				v.assign(other, 2, 2);
				AssertThat(v, Equals("pl"));
				StringView sv{"KiwiApple"};
				v.assign(sv, 4, 5);
				AssertThat(v, Equals("Apple"));
				v.assign({'a', 'b', 'c'});
				AssertThat(v, Equals("abc"));
			});

			it("Can self assign", [&]()
			{
				String v{"Kiwi"};
				const String& ref = v;
				v                 = ref;
				AssertThat(v, Equals("Kiwi"));
			});

			it("Can self assign substrings", [&]()
			{
				String v{longText};
				v.assign(v.c_str() + 10);
				AssertThat(v, Equals("ABCDEFGHIJ0123456789ABC"));
			});

			it("Can self assign substrings with count", [&]()
			{
				String v{longText};
				v.assign(v.c_str() + 5, 10);
				AssertThat(v, Equals("56789ABCDE"));
			});
		});

		describe("Element access", []()
		{
			it("Can index", [&]()
			{
				String v{"Kiwi"};
				AssertThat(v[0], Equals('K'));
				AssertThat(v[3], Equals('i'));
				v[0] = 'k';
				AssertThat(v, Equals("kiwi"));
				// pos == size() returns reference to null char
				AssertThat(v[4], Equals('\0'));
			});

			it("Can access at", [&]()
			{
				String v{"Kiwi"};
				AssertThat(v.at(0), Equals('K'));
				AssertThat(v.at(3), Equals('i'));
				v.at(0) = 'k';
				AssertThat(v, Equals("kiwi"));
			});

			it("Can access front and back", [&]()
			{
				String v{"Kiwi"};
				AssertThat(v.front(), Equals('K'));
				AssertThat(v.back(), Equals('i'));
				v.front() = 'P';
				v.back()  = 's';
				AssertThat(v, Equals("Piws"));
			});

			it("Can retrieve data", [&]()
			{
				String v{"Kiwi"};
				AssertThat(v.data(), Equals("Kiwi"));
				AssertThat(v.size(), Equals(4u));
				AssertThat(strlen(v.data()), Equals(4u));
			});

			it("Can convert to string view", [&]()
			{
				String v{"Kiwi"};
				StringView sv = v;
				AssertThat(sv.size(), Equals(4u));
				AssertThat(sv, Equals(StringView{"Kiwi"}));
				StringView wsv{v};
				AssertThat(wsv, Equals(StringView{"Kiwi"}));
			});
		});

		describe("Iterators", []()
		{
			it("Can iterate", [&]()
			{
				String v{"Kiwi"};
				u32 i = 0;
				for (char c : v)
				{
					AssertThat(c, Equals("Kiwi"[i]));
					++i;
				}
				AssertThat(i, Equals(4u));
			});

			it("Can iterate const", [&]()
			{
				const String v{"Kiwi"};
				u32 i = 0;
				for (char c : v)
				{
					AssertThat(c, Equals("Kiwi"[i]));
					++i;
				}
				AssertThat(i, Equals(4u));
			});

			it("Can iterate manually", [&]()
			{
				String v{"Kiwi"};
				auto it  = v.begin();
				auto end = v.end();
				AssertThat(end - it, Equals(4));
				AssertThat(*it, Equals('K'));
				AssertThat(it[2], Equals('w'));
				++it;
				AssertThat(*it, Equals('i'));
				it += 2;
				AssertThat(*it, Equals('i'));
				--it;
				AssertThat(*it, Equals('w'));
				AssertThat(it == v.begin() + 2, Is().True());
				AssertThat(it != v.begin(), Is().True());
			});

			it("Can iterate reverse", [&]()
			{
				String v{"Kiwi"};
				u32 i = 0;
				for (auto rit = v.rbegin(); rit != v.rend(); ++rit)
				{
					AssertThat(*rit, Equals("Kiwi"[3 - i]));
					++i;
				}
				AssertThat(i, Equals(4u));
			});

			it("Can iterate c-variants", [&]()
			{
				String v{"Kiwi"};
				AssertThat(*v.cbegin(), Equals('K'));
				AssertThat(*(v.cend() - 1), Equals('i'));
				AssertThat(*v.crbegin(), Equals('i'));
				AssertThat(*(v.crend() - 1), Equals('K'));
			});

			it("Can mutate through iterators", [&]()
			{
				String v{"Kiwi"};
				std::transform(v.begin(), v.end(), v.begin(), [](char c)
				{
					return char(c + 1);
				});
				AssertThat(v, Equals("Ljxj"));
			});
		});

		describe("Capacity", []()
		{
			it("Can query size and length", [&]()
			{
				String v{"Kiwi"};
				AssertThat(v.size(), Equals(4u));
				AssertThat(v.length(), Equals(4u));
				AssertThat(v.empty(), Is().False());
			});

			it("Has short string optimization", [&]()
			{
				String v{"Kiwi"};
				// Short strings must fit in the internal buffer
				AssertThat(v.capacity() >= 15u, Is().True());
				AssertThat(v.capacity() <= 32u, Is().True());
			});

			it("Can reserve", [&]()
			{
				String v;
				v.reserve(100);
				AssertThat(v.capacity() >= 100u, Is().True());
				AssertThat(v.size(), Equals(0u));
				v = "Kiwi";
				AssertThat(v, Equals("Kiwi"));
				AssertThat(v.capacity() >= 100u, Is().True());
			});

			it("Can shrink to fit", [&]()
			{
				String v;
				v.reserve(100);
				v = "Kiwi";
				v.shrink_to_fit();
				AssertThat(v, Equals("Kiwi"));
				AssertThat(v.capacity() >= 4u, Is().True());
				AssertThat(v.capacity() < 100u, Is().True());
			});

			it("Has max size", [&]()
			{
				String v;
				// Lengths are stored internally as i32
				AssertThat(v.max_size(), Equals(sizet(Limits<i32>::Max() - 1)));
			});
		});

		describe("Modifiers", []()
		{
			it("Can clear", [&]()
			{
				String v{"Kiwi"};
				v.clear();
				AssertThat(v.empty(), Is().True());
				AssertThat(v.size(), Equals(0u));
				AssertThat(v.c_str()[0], Equals('\0'));
			});

			it("Can push and pop back", [&]()
			{
				String v{"Ki"};
				v.push_back('w');
				v.push_back('i');
				AssertThat(v, Equals("Kiwi"));
				AssertThat(v.back(), Equals('i'));
				v.pop_back();
				AssertThat(v, Equals("Kiw"));
				v.pop_back();
				v.pop_back();
				v.pop_back();
				AssertThat(v, Equals(""));
				AssertThat(v.empty(), Is().True());
			});

			it("Can append", [&]()
			{
				String v{"Kiwi"};
				v.append("Apple");
				AssertThat(v, Equals("KiwiApple"));
				v.append("Orange", 3);
				AssertThat(v, Equals("KiwiAppleOra"));
				v.append(3, '-');
				AssertThat(v, Equals("KiwiAppleOra---"));
				String other{"End"};
				v.append(other);
				AssertThat(v, Equals("KiwiAppleOra---End"));
				v.append(other, 1, 2);
				AssertThat(v, Equals("KiwiAppleOra---Endnd"));
				StringView sv{"View"};
				v.append(sv);
				AssertThat(v, Equals("KiwiAppleOra---EndndView"));
				v.append(sv, 2, 2);
				AssertThat(v, Equals("KiwiAppleOra---EndndViewew"));
				v.append({'!', '?'});
				AssertThat(v, Equals("KiwiAppleOra---EndndViewew!?"));
			});

			it("Can append with operator+=", [&]()
			{
				String v{"Kiwi"};
				v += "Apple";
				AssertThat(v, Equals("KiwiApple"));
				v += '!';
				AssertThat(v, Equals("KiwiApple!"));
				String other{"End"};
				v += other;
				AssertThat(v, Equals("KiwiApple!End"));
				v += StringView{"View"};
				AssertThat(v, Equals("KiwiApple!EndView"));
				v += {'a', 'b'};
				AssertThat(v, Equals("KiwiApple!EndViewab"));
			});

			it("Can insert", [&]()
			{
				String v{"KiwiApple"};
				v.insert(4, "Orange");
				AssertThat(v, Equals("KiwiOrangeApple"));
				v.insert(0, "-");
				AssertThat(v, Equals("-KiwiOrangeApple"));
				v.insert(v.size(), "!");
				AssertThat(v, Equals("-KiwiOrangeApple!"));
				v.insert(0, 3, '=');
				AssertThat(v, Equals("===-KiwiOrangeApple!"));
				String other{"XX"};
				v.insert(3, other);
				AssertThat(v, Equals("===XX-KiwiOrangeApple!"));
				StringView sv{"YY"};
				v.insert(5, sv);
				AssertThat(v, Equals("===XXYY-KiwiOrangeApple!"));
				v.insert(0, 2, 'Z');
				AssertThat(v, Equals("ZZ===XXYY-KiwiOrangeApple!"));
			});

			it("Can insert with iterator", [&]()
			{
				String v{"Kiwi"};
				auto it = v.insert(v.begin() + 2, '-');
				AssertThat(*it, Equals('-'));
				AssertThat(v, Equals("Ki-wi"));
				v.insert(v.end(), 3, '!');
				AssertThat(v, Equals("Ki-wi!!!"));
				String other{"AB"};
				v.insert(v.begin(), other.begin(), other.end());
				AssertThat(v, Equals("ABKi-wi!!!"));
				v.insert(v.begin() + 2, {'x', 'y'});
				AssertThat(v, Equals("ABxyKi-wi!!!"));
			});

			it("Can erase", [&]()
			{
				String v{"KiwiApple"};
				v.erase(4, 5);
				AssertThat(v, Equals("Kiwi"));
				v.erase(2);
				AssertThat(v, Equals("Ki"));
				v.erase(0, 1);
				AssertThat(v, Equals("i"));
				v.erase(0, 10);
				AssertThat(v, Equals(""));
			});

			it("Can erase with iterator", [&]()
			{
				String v{"Kiwi"};
				auto it = v.erase(v.begin());
				AssertThat(*it, Equals('i'));
				AssertThat(v, Equals("iwi"));
				v.erase(v.begin() + 1, v.end());
				AssertThat(v, Equals("i"));
			});

			it("Can replace", [&]()
			{
				String v{"KiwiApple"};
				v.replace(0, 4, "Orange");
				AssertThat(v, Equals("OrangeApple"));
				v.replace(0, 6, "X");
				AssertThat(v, Equals("XApple"));
				v.replace(v.size() - 3, 3, "Z");
				AssertThat(v, Equals("XApZ"));
				String other{"Kiwi"};
				v.replace(0, 4, other);
				AssertThat(v, Equals("Kiwi"));
				StringView sv{"Two"};
				v.replace(0, 4, sv);
				AssertThat(v, Equals("Two"));
				v.replace(0, 3, 2, 'y');
				AssertThat(v, Equals("yy"));
			});

			it("Can replace with iterators", [&]()
			{
				String v{"KiwiApple"};
				v.replace(v.begin(), v.begin() + 4, "Orange");
				AssertThat(v, Equals("OrangeApple"));
			});

			it("Can resize", [&]()
			{
				String v{"Kiwi"};
				v.resize(2);
				AssertThat(v, Equals("Ki"));
				v.resize(4);
				AssertThat(v.size(), Equals(4u));
				AssertThat(v[2], Equals('\0'));
				AssertThat(v[3], Equals('\0'));
				v.resize(6, 'x');
				AssertThat(v[4], Equals('x'));
				AssertThat(v[5], Equals('x'));
				AssertThat(v.size(), Equals(6u));
			});

			it("Can swap", [&]()
			{
				String a{"Kiwi"};
				String b{"Apple"};
				a.swap(b);
				AssertThat(a, Equals("Apple"));
				AssertThat(b, Equals("Kiwi"));
			});

			it("Can append from self", [&]()
			{
				String v{longText};
				v.append(v.c_str());
				AssertThat(v, Equals(std::string{longText} + std::string{longText}));
			});

			it("Can append self substring", [&]()
			{
				String v{longText};
				v.append(v.c_str() + 5);
				AssertThat(v, Equals(std::string{longText} + std::string{longText.substr(5)}));
			});

			it("Can insert from self", [&]()
			{
				String v{longText};
				v.insert(0, v.c_str());
				AssertThat(v, Equals(std::string{longText} + std::string{longText}));
			});

			it("Can insert self substring", [&]()
			{
				String v{longText};
				v.insert(4, v.c_str() + 5);
				AssertThat(
				    v, Equals(std::string{longText.substr(0, 4)} + std::string{longText.substr(5)}
				              + std::string{longText.substr(4)}));
			});

			it("Can replace with self", [&]()
			{
				String v{longText};
				v.replace(0, 4, v.c_str());
				AssertThat(v, Equals(std::string{longText} + std::string{longText.substr(4)}));
			});

			it("Can replace self substring with count", [&]()
			{
				String v{longText};
				v.replace(5, 10, v.c_str() + 2, 5);
				AssertThat(v, Equals(std::string{longText.substr(0, 5)} + "23456"
				                     + std::string{longText.substr(15)}));
			});
		});

		describe("Operations", []()
		{
			it("Can get substr", [&]()
			{
				String v{"KiwiApple"};
				AssertThat(v.substr(), Equals("KiwiApple"));
				AssertThat(v.substr(4), Equals("Apple"));
				AssertThat(v.substr(4, 3), Equals("App"));
				AssertThat(v.substr(0, 100), Equals("KiwiApple"));
			});

			it("Can copy out", [&]()
			{
				String v{"KiwiApple"};
				char buffer[16]{};
				const auto count = v.copy(buffer, 4, 4);
				AssertThat(count, Equals(4u));
				AssertThat(buffer, Equals("Appl"));
				buffer[count] = '\0';
			});

			it("Can compare", [&]()
			{
				String v{"Kiwi"};
				String other{"Kiwi"};
				String apple{"Apple"};
				AssertThat(v.compare(other), Equals(0));
				AssertThat(v.compare(apple) > 0, Is().True());
				AssertThat(apple.compare(v) < 0, Is().True());
				AssertThat(v.compare("Kiwi"), Equals(0));
				AssertThat(v.compare("Kiwi2") < 0, Is().True());
				AssertThat(v.compare(StringView{"Kiwi"}), Equals(0));
				AssertThat(v.compare(0, 2, String{"Ki"}), Equals(0));
				AssertThat(v.compare(2, 2, String{"wi"}), Equals(0));
			});

			it("Can check prefix and suffix", [&]()
			{
				String v{"KiwiApple"};
				AssertThat(v.starts_with("Kiwi"), Is().True());
				AssertThat(v.starts_with('K'), Is().True());
				AssertThat(v.starts_with(StringView{"Ki"}), Is().True());
				AssertThat(v.starts_with("Apple"), Is().False());
				AssertThat(v.ends_with("Apple"), Is().True());
				AssertThat(v.ends_with('e'), Is().True());
				AssertThat(v.ends_with(StringView{"le"}), Is().True());
				AssertThat(v.ends_with("Kiwi"), Is().False());
			});

			it("Can check contains", [&]()
			{
				String v{"KiwiApple"};
				AssertThat(v.contains("wiA"), Is().True());
				AssertThat(v.contains('A'), Is().True());
				AssertThat(v.contains(StringView{"zzz"}), Is().False());
				AssertThat(v.contains('z'), Is().False());
			});

			it("Can find", [&]()
			{
				String v{"KiwiKiwi"};
				AssertThat(v.find("Kiwi"), Equals(0u));
				AssertThat(v.find("Kiwi", 1), Equals(4u));
				AssertThat(v.find("Kiwi", 5), Equals(String::npos));
				AssertThat(v.find('i'), Equals(1u));
				AssertThat(v.find('i', 6), Equals(7u));
				AssertThat(v.find('z'), Equals(String::npos));
				AssertThat(v.find(String{"Kiwi"}), Equals(0u));
				AssertThat(v.find(StringView{"Kiwi"}), Equals(0u));
			});

			it("Can rfind", [&]()
			{
				String v{"KiwiKiwi"};
				AssertThat(v.rfind("Kiwi"), Equals(4u));
				AssertThat(v.rfind("Kiwi", 3), Equals(0u));
				AssertThat(v.rfind('i'), Equals(7u));
				AssertThat(v.rfind('i', 5), Equals(5u));
				AssertThat(v.rfind('z'), Equals(String::npos));
				AssertThat(v.rfind(String{"Kiwi"}), Equals(4u));
				AssertThat(v.rfind(StringView{"Kiwi"}), Equals(4u));
			});

			it("Can find first of", [&]()
			{
				String v{"KiwiApple"};
				AssertThat(v.find_first_of("pl"), Equals(5u));
				AssertThat(v.find_first_of("pl", 6), Equals(6u));
				AssertThat(v.find_first_of('z'), Equals(String::npos));
				AssertThat(v.find_first_of("xyz"), Equals(String::npos));
				AssertThat(v.find_first_of(StringView{"Ap"}), Equals(4u));
			});

			it("Can find last of", [&]()
			{
				String v{"KiwiApple"};
				AssertThat(v.find_last_of("pl"), Equals(7u));
				AssertThat(v.find_last_of("pl", 6), Equals(6u));
				AssertThat(v.find_last_of('z'), Equals(String::npos));
				AssertThat(v.find_last_of(StringView{"Ap"}), Equals(6u));
			});

			it("Can find first not of", [&]()
			{
				String v{"aaab"};
				AssertThat(v.find_first_not_of("a"), Equals(3u));
				AssertThat(v.find_first_not_of("ab"), Equals(String::npos));
				AssertThat(v.find_first_not_of('a'), Equals(3u));
				AssertThat(v.find_first_not_of("ab", 3), Equals(String::npos));
			});

			it("Can find last not of", [&]()
			{
				String v{"baaa"};
				AssertThat(v.find_last_not_of("a"), Equals(0u));
				AssertThat(v.find_last_not_of("ab"), Equals(String::npos));
				AssertThat(v.find_last_not_of('a'), Equals(0u));
				AssertThat(v.find_last_not_of("ab", 0), Equals(String::npos));
			});

			it("Has npos", [&]()
			{
				AssertThat(String::npos, Equals(sizet(-1)));
				AssertThat(StringView::npos, Equals(String::npos));
			});
		});

		describe("Operators", []()
		{
			it("Can concatenate", [&]()
			{
				String a{"Kiwi"};
				String b{"Apple"};
				AssertThat(a + b, Equals("KiwiApple"));
				AssertThat(a + "X", Equals("KiwiX"));
				AssertThat("X" + a, Equals("XKiwi"));
				AssertThat(a + '!', Equals("Kiwi!"));
				AssertThat('!' + a, Equals("!Kiwi"));
				AssertThat(a + StringView{"V"}, Equals("KiwiV"));
				AssertThat(StringView{"V"} + a, Equals("VKiwi"));
			});

			it("Can chain concatenate", [&]()
			{
				String a{"Kiwi"};
				String result = a + " " + "Apple" + '!';
				AssertThat(result, Equals("Kiwi Apple!"));
			});

			it("Can compare with other types", [&]()
			{
				String v{"Kiwi"};
				AssertThat(v == String{"Kiwi"}, Is().True());
				AssertThat(v != String{"Apple"}, Is().True());
				AssertThat(v == "Kiwi", Is().True());
				AssertThat(v != "Apple", Is().True());
				AssertThat("Kiwi" == v, Is().True());
				AssertThat("Apple" != v, Is().True());
				AssertThat(v < "Lime", Is().True());
				AssertThat("Lime" > v, Is().True());
				AssertThat(v <= String{"Kiwi"}, Is().True());
				AssertThat(v >= String{"Kiwi"}, Is().True());
				AssertThat(v == StringView{"Kiwi"}, Is().True());
				AssertThat(StringView{"Kiwi"} == v, Is().True());
				AssertThat(v != StringView{"Apple"}, Is().True());
				AssertThat(StringView{"Apple"} != v, Is().True());
				AssertThat(v < StringView{"Lime"}, Is().True());
				AssertThat(StringView{"Lime"} > v, Is().True());
			});

			it("Can three-way compare", [&]()
			{
				String a{"Kiwi"};
				String b{"Lime"};
				AssertThat((a <=> b) < 0, Is().True());
				AssertThat((b <=> a) > 0, Is().True());
				AssertThat((a <=> String{"Kiwi"}) == 0, Is().True());
				AssertThat((a <=> "Kiwi") == 0, Is().True());
			});
		});

		describe("Memory", []()
		{
			it("Keeps data valid when growing", [&]()
			{
				String v;
				for (char c = 'a'; c <= 'z'; ++c)
				{
					v.push_back(c);
				}
				AssertThat(v.size(), Equals(26u));
				AssertThat(v, Equals("abcdefghijklmnopqrstuvwxyz"));
				AssertThat(v.c_str()[26], Equals('\0'));
			});

			it("Can reuse capacity", [&]()
			{
				String v;
				v.reserve(1000);
				const auto cap = v.capacity();
				for (u32 i = 0; i < 100; ++i)
				{
					v.assign("KiwiAppleOrangeBanana");
					v.clear();
				}
				AssertThat(v.capacity(), Equals(cap));
			});

			it("Is valid after move assignment", [&]()
			{
				String a{"Kiwi"};
				String b;
				b = Move(a);
				AssertThat(b, Equals("Kiwi"));
				a = "Reused";
				AssertThat(a, Equals("Reused"));
			});
		});

		describe("Format & Hash", []()
		{
			it("Can be formatted", [&]()
			{
				String v{"Kiwi"};
				AssertThat(std::format("{}", v), Equals("Kiwi"));
				AssertThat(Format("{}-{}", v, 5), Equals("Kiwi-5"));
				String out;
				FormatTo(out, "{}!", v);
				AssertThat(out, Equals("Kiwi!"));
			});

			it("Can be hashed", [&]()
			{
				String v{"Kiwi"};
				AssertThat(GetHash(v), Equals(GetStringHash("Kiwi")));
				AssertThat(GetHash(StringView{"Kiwi"}), Equals(GetHash(v)));
			});
		});

		describe("Arena", []()
		{
			const char* longText = "This string is long enough to exceed the inline capacity";

			it("Can default construct on an arena", [&]()
			{
				MonoLinearArena arena{Memory::KB * 4};
				String v{arena};
				AssertThat(v.empty(), Is().True());
				AssertThat(&v.GetArena(), Equals(static_cast<Arena*>(&arena)));
				// Short strings still use the inline buffer
				v = "Kiwi";
				AssertThat(v, Equals("Kiwi"));
				AssertThat(v.capacity() <= 32u, Is().True());
			});

			it("Can allocate on an arena", [&]()
			{
				MonoLinearArena arena{Memory::KB * 4};
				String v{arena, longText};
				AssertThat(v, Equals(longText));
				AssertThat(&v.GetArena(), Equals(static_cast<Arena*>(&arena)));
				// Long strings must allocate on the arena, not the current arena
				AssertThat(v.capacity() >= v.size(), Is().True());
			});

			it("Can construct with count and char on an arena", [&]()
			{
				MonoLinearArena arena{Memory::KB * 4};
				String v{arena, 64, 'x'};
				AssertThat(v.size(), Equals(64u));
				AssertThat(&v.GetArena(), Equals(static_cast<Arena*>(&arena)));
			});

			it("Can copy into an arena", [&]()
			{
				MonoLinearArena arena{Memory::KB * 4};
				String original{longText};
				String v{arena, original};
				AssertThat(v, Equals(original));
				AssertThat(&v.GetArena(), Equals(static_cast<Arena*>(&arena)));
			});

			it("Keeps its arena when assigned", [&]()
			{
				MonoLinearArena arena{Memory::KB * 4};
				String v{arena};
				v.assign(longText);
				v.append(" with some extra content to force a reallocation");
				AssertThat(&v.GetArena(), Equals(static_cast<Arena*>(&arena)));
				AssertThat(v.starts_with("This string"), Is().True());
			});
		});

		describe("Strings helpers", []()
		{
			it("RemoveFromStart", [&]()
			{
				String v{"KiwiApple"};
				Strings::RemoveFromStart(v, 4);
				AssertThat(v, Equals("Apple"));
				Strings::RemoveFromStart(v, 100);
				AssertThat(v.empty(), Is().True());
			});

			it("RemoveFromEnd", [&]()
			{
				String v{"KiwiApple"};
				Strings::RemoveFromEnd(v, 5);
				AssertThat(v, Equals("Kiwi"));
				Strings::RemoveFromEnd(v, StringView{"wi"});
				AssertThat(v, Equals("Ki"));
				Strings::RemoveFromEnd(v, 100);
				AssertThat(v.empty(), Is().True());
			});

			it("RemoveCharFromEnd", [&]()
			{
				String v{"Kiwi!"};
				AssertThat(Strings::RemoveCharFromEnd(v, '!'), Is().True());
				AssertThat(v, Equals("Kiwi"));
				AssertThat(Strings::RemoveCharFromEnd(v, '!'), Is().False());
				AssertThat(v, Equals("Kiwi"));
			});

			it("ToSentenceCase", [&]()
			{
				AssertThat(Strings::ToSentenceCase(""), Equals(""));
				AssertThat(Strings::ToSentenceCase("papa"), Equals("Papa"));
				AssertThat(Strings::ToSentenceCase("papa "), Equals("Papa "));
				AssertThat(Strings::ToSentenceCase("papa3"), Equals("Papa 3"));
				AssertThat(Strings::ToSentenceCase("MisterPotato"), Equals("Mister Potato"));
			});

			it("Convert u16 to u8", [&]()
			{
				TString<Char16> utf16string{0x41, 0x0448, 0x65e5, 0xd834, 0xdd1e};
				TString<Char8> u = Strings::Convert<TString<Char8>>(utf16string);
				AssertThat(u.size(), Equals(10u));
			});
			it("Convert u8 to u16", [&]()
			{
				TString<AnsiChar> utf8_with_surrogates = "\xe6\x97\xa5\xd1\x88\xf0\x9d\x84\x9e";
				TString<Char16> utf16result =
				    Strings::Convert<TString<Char16>>(utf8_with_surrogates);
				AssertThat(utf16result.size(), Equals(4u));
				AssertThat(utf16result[2] == 0xd834, Is().True());
				AssertThat(utf16result[3] == 0xdd1e, Is().True());
			});
			it("Convert u32 to u8", [&]()
			{
				TString<Char32> utf32string = {0x448, 0x65E5, 0x10346};
				TString<Char8> utf8result   = Strings::Convert<TString<Char8>>(utf32string);
				AssertThat(utf8result.size(), Equals(9u));
			});
			it("Convert u8 to u32", [&]()
			{
				TString<AnsiChar> twochars  = "\xe6\x97\xa5\xd1\x88";
				TString<Char32> utf32result = Strings::Convert<TString<Char32>>(twochars);
				AssertThat(utf32result.size(), Equals(2u));
			});
		});
	});
});
