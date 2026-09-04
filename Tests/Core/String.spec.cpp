// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <Pipe/Core/StringView.h>
#include <PipeMemoryArenas.h>
#include <PipeStrings.h>

#include <algorithm>
#include <format>


using namespace p;

// Longer than the inline capacity, forcing heap allocations
static const StringView longText = "0123456789ABCDEFGHIJ0123456789ABC";

// Longer than the inline capacity, for arena allocation tests
static const char* arenaLongText = "This string is long enough to exceed the inline capacity";


namespace
{
// Auto-registers via static init (macro-free go_bandit equivalent).
const bool autoRegistered = []()
{
Spec("Strings", []()
{
	Describe("String", []()
	{
		Describe("Construction", []()
		{
			It("Can default construct", []()
			{
				String v{};
				Expect(v.size()).ToEqual(0u);
				Expect(v.empty()).ToBeTrue();
				Expect(v.length()).ToEqual(0u);
				// c_str() must always return a valid pointer to a null terminator
				Expect(v.c_str() != nullptr).ToBeTrue();
				Expect(v.c_str()[0]).ToEqual('\0');
				Expect(v.data() != nullptr).ToBeTrue();
				Expect(v.data()[0]).ToEqual('\0');
			});

			It("Can construct from literal", []()
			{
				String v{"Kiwi"};
				Expect(v).ToEqual("Kiwi");
				Expect(v.size()).ToEqual(4u);
			});

			It("Can construct from literal with count", []()
			{
				String v{"KiwiApple", 4};
				Expect(v).ToEqual("Kiwi");
				Expect(v.size()).ToEqual(4u);
			});

			It("Can construct from count and char", []()
			{
				String v(5, 'x');
				Expect(v).ToEqual("xxxxx");
				Expect(v.size()).ToEqual(5u);
			});

			It("Can construct from string view", []()
			{
				StringView str{"Kiwi"};
				String v{str};
				Expect(v).ToEqual("Kiwi");
				Expect(v.size()).ToEqual(4u);
			});

			It("Can construct from string view with pos and count", []()
			{
				StringView str{"KiwiApple"};
				String v{str, 4, 5};
				Expect(v).ToEqual("Apple");
			});

			It("Can construct from substring", []()
			{
				String str{"KiwiApple"};
				String v{str, 4};
				Expect(v).ToEqual("Apple");
				String v2{str, 4, 3};
				Expect(v2).ToEqual("App");
			});

			It("Can construct from iterators", []()
			{
				std::string_view sv = "Kiwi";
				String v{sv.begin(), sv.end()};
				Expect(v).ToEqual("Kiwi");
			});

			It("Can construct from initializer list", []()
			{
				String v{'K', 'i', 'w', 'i'};
				Expect(v).ToEqual("Kiwi");
			});

			It("Can copy construct", []()
			{
				String v{"Kiwi"};
				String v2{v};
				Expect(v2).ToEqual("Kiwi");
				Expect(v).ToEqual("Kiwi");
			});

			It("Can move construct", []()
			{
				String v{"Kiwi"};
				String v2{Move(v)};
				Expect(v2).ToEqual("Kiwi");
				// Moved-from string is valid and empty
				Expect(v.size()).ToEqual(0u);
				Expect(v.empty()).ToBeTrue();
				Expect(v.c_str()[0]).ToEqual('\0');
			});
		});

		Describe("Assignment", []()
		{
			It("Can assign from literal", []()
			{
				String v;
				v = "Kiwi";
				Expect(v).ToEqual("Kiwi");
			});

			It("Can copy assign", []()
			{
				String vKiwi{"Kiwi"};
				String vApple{"Apple"};
				String vCopy = vKiwi;
				Expect(vCopy).ToEqual("Kiwi");
				vCopy = vApple;
				Expect(vCopy).ToEqual("Apple");
				Expect(vCopy).ToEqual(vApple);
			});

			It("Can move assign", []()
			{
				String vKiwi{"Kiwi"};
				String vApple{"Apple"};
				String vMove = Move(vKiwi);
				Expect(vKiwi.size()).ToEqual(0u);
				Expect(vMove).ToEqual("Kiwi");
				vMove = Move(vApple);
				Expect(vApple.size()).ToEqual(0u);
				Expect(vMove).ToEqual("Apple");
			});

			It("Can assign char", []()
			{
				String v;
				v = 'x';
				Expect(v).ToEqual("x");
			});

			It("Can assign initializer list", []()
			{
				String v;
				v = {'K', 'i', 'w', 'i'};
				Expect(v).ToEqual("Kiwi");
			});

			It("Can assign string view", []()
			{
				String v;
				StringView sv{"Kiwi"};
				v = sv;
				Expect(v).ToEqual("Kiwi");
			});

			It("Can assign", []()
			{
				String v;
				v.assign("Kiwi");
				Expect(v).ToEqual("Kiwi");
				v.assign("KiwiApple", 4);
				Expect(v).ToEqual("Kiwi");
				v.assign(3, 'x');
				Expect(v).ToEqual("xxx");
				String other{"Apple"};
				v.assign(other);
				Expect(v).ToEqual("Apple");
				v.assign(other, 2, 2);
				Expect(v).ToEqual("pl");
				StringView sv{"KiwiApple"};
				v.assign(sv, 4, 5);
				Expect(v).ToEqual("Apple");
				v.assign({'a', 'b', 'c'});
				Expect(v).ToEqual("abc");
			});

			It("Can self assign", []()
			{
				String v{"Kiwi"};
				const String& ref = v;
				v                 = ref;
				Expect(v).ToEqual("Kiwi");
			});

			It("Can self assign substrings", []()
			{
				String v{longText};
				v.assign(v.c_str() + 10);
				Expect(v).ToEqual("ABCDEFGHIJ0123456789ABC");
			});

			It("Can self assign substrings with count", []()
			{
				String v{longText};
				v.assign(v.c_str() + 5, 10);
				Expect(v).ToEqual("56789ABCDE");
			});
		});

		Describe("Element access", []()
		{
			It("Can index", []()
			{
				String v{"Kiwi"};
				Expect(v[0]).ToEqual('K');
				Expect(v[3]).ToEqual('i');
				v[0] = 'k';
				Expect(v).ToEqual("kiwi");
				// pos == size() returns reference to null char
				Expect(v[4]).ToEqual('\0');
			});

			It("Can access at", []()
			{
				String v{"Kiwi"};
				Expect(v.at(0)).ToEqual('K');
				Expect(v.at(3)).ToEqual('i');
				v.at(0) = 'k';
				Expect(v).ToEqual("kiwi");
			});

			It("Can access front and back", []()
			{
				String v{"Kiwi"};
				Expect(v.front()).ToEqual('K');
				Expect(v.back()).ToEqual('i');
				v.front() = 'P';
				v.back()  = 's';
				Expect(v).ToEqual("Piws");
			});

			It("Can retrieve data", []()
			{
				String v{"Kiwi"};
				Expect(v.data()).ToEqual("Kiwi");
				Expect(v.size()).ToEqual(4u);
				Expect(strlen(v.data())).ToEqual(4u);
			});

			It("Can convert to string view", []()
			{
				String v{"Kiwi"};
				StringView sv = v;
				Expect(sv.size()).ToEqual(4u);
				Expect(sv).ToEqual(StringView{"Kiwi"});
				StringView wsv{v};
				Expect(wsv).ToEqual(StringView{"Kiwi"});
			});
		});

		Describe("Iterators", []()
		{
			It("Can iterate", []()
			{
				String v{"Kiwi"};
				u32 i = 0;
				for (char c : v)
				{
					Expect(c).ToEqual("Kiwi"[i]);
					++i;
				}
				Expect(i).ToEqual(4u);
			});

			It("Can iterate const", []()
			{
				const String v{"Kiwi"};
				u32 i = 0;
				for (char c : v)
				{
					Expect(c).ToEqual("Kiwi"[i]);
					++i;
				}
				Expect(i).ToEqual(4u);
			});

			It("Can iterate manually", []()
			{
				String v{"Kiwi"};
				auto it  = v.begin();
				auto end = v.end();
				Expect(end - it).ToEqual(4);
				Expect(*it).ToEqual('K');
				Expect(it[2]).ToEqual('w');
				++it;
				Expect(*it).ToEqual('i');
				it += 2;
				Expect(*it).ToEqual('i');
				--it;
				Expect(*it).ToEqual('w');
				Expect(it == v.begin() + 2).ToBeTrue();
				Expect(it != v.begin()).ToBeTrue();
			});

			It("Can iterate reverse", []()
			{
				String v{"Kiwi"};
				u32 i = 0;
				for (auto rit = v.rbegin(); rit != v.rend(); ++rit)
				{
					Expect(*rit).ToEqual("Kiwi"[3 - i]);
					++i;
				}
				Expect(i).ToEqual(4u);
			});

			It("Can iterate c-variants", []()
			{
				String v{"Kiwi"};
				Expect(*v.cbegin()).ToEqual('K');
				Expect(*(v.cend() - 1)).ToEqual('i');
				Expect(*v.crbegin()).ToEqual('i');
				Expect(*(v.crend() - 1)).ToEqual('K');
			});

			It("Can mutate through iterators", []()
			{
				String v{"Kiwi"};
				std::transform(v.begin(), v.end(), v.begin(), [](char c)
				{
					return char(c + 1);
				});
				Expect(v).ToEqual("Ljxj");
			});
		});

		Describe("Capacity", []()
		{
			It("Can query size and length", []()
			{
				String v{"Kiwi"};
				Expect(v.size()).ToEqual(4u);
				Expect(v.length()).ToEqual(4u);
				Expect(v.empty()).ToBeFalse();
			});

			It("Has short string optimization", []()
			{
				String v{"Kiwi"};
				// Short strings must fit in the internal buffer
				Expect(v.capacity() >= 15u).ToBeTrue();
				Expect(v.capacity() <= 32u).ToBeTrue();
			});

			It("Can reserve", []()
			{
				String v;
				v.reserve(100);
				Expect(v.capacity() >= 100u).ToBeTrue();
				Expect(v.size()).ToEqual(0u);
				v = "Kiwi";
				Expect(v).ToEqual("Kiwi");
				Expect(v.capacity() >= 100u).ToBeTrue();
			});

			It("Can shrink to fit", []()
			{
				String v;
				v.reserve(100);
				v = "Kiwi";
				v.shrink_to_fit();
				Expect(v).ToEqual("Kiwi");
				Expect(v.capacity() >= 4u).ToBeTrue();
				Expect(v.capacity() < 100u).ToBeTrue();
			});

			It("Has max size", []()
			{
				String v;
				// Lengths are stored internally as i32
				Expect(v.max_size()).ToEqual(sizet(Limits<i32>::Max() - 1));
			});
		});

		Describe("Modifiers", []()
		{
			It("Can clear", []()
			{
				String v{"Kiwi"};
				v.clear();
				Expect(v.empty()).ToBeTrue();
				Expect(v.size()).ToEqual(0u);
				Expect(v.c_str()[0]).ToEqual('\0');
			});

			It("Can push and pop back", []()
			{
				String v{"Ki"};
				v.push_back('w');
				v.push_back('i');
				Expect(v).ToEqual("Kiwi");
				Expect(v.back()).ToEqual('i');
				v.pop_back();
				Expect(v).ToEqual("Kiw");
				v.pop_back();
				v.pop_back();
				v.pop_back();
				Expect(v).ToEqual("");
				Expect(v.empty()).ToBeTrue();
			});

			It("Can append", []()
			{
				String v{"Kiwi"};
				v.append("Apple");
				Expect(v).ToEqual("KiwiApple");
				v.append("Orange", 3);
				Expect(v).ToEqual("KiwiAppleOra");
				v.append(3, '-');
				Expect(v).ToEqual("KiwiAppleOra---");
				String other{"End"};
				v.append(other);
				Expect(v).ToEqual("KiwiAppleOra---End");
				v.append(other, 1, 2);
				Expect(v).ToEqual("KiwiAppleOra---Endnd");
				StringView sv{"View"};
				v.append(sv);
				Expect(v).ToEqual("KiwiAppleOra---EndndView");
				v.append(sv, 2, 2);
				Expect(v).ToEqual("KiwiAppleOra---EndndViewew");
				v.append({'!', '?'});
				Expect(v).ToEqual("KiwiAppleOra---EndndViewew!?");
			});

			It("Can append with operator+=", []()
			{
				String v{"Kiwi"};
				v += "Apple";
				Expect(v).ToEqual("KiwiApple");
				v += '!';
				Expect(v).ToEqual("KiwiApple!");
				String other{"End"};
				v += other;
				Expect(v).ToEqual("KiwiApple!End");
				v += StringView{"View"};
				Expect(v).ToEqual("KiwiApple!EndView");
				v += {'a', 'b'};
				Expect(v).ToEqual("KiwiApple!EndViewab");
			});

			It("Can insert", []()
			{
				String v{"KiwiApple"};
				v.insert(4, "Orange");
				Expect(v).ToEqual("KiwiOrangeApple");
				v.insert(0, "-");
				Expect(v).ToEqual("-KiwiOrangeApple");
				v.insert(v.size(), "!");
				Expect(v).ToEqual("-KiwiOrangeApple!");
				v.insert(0, 3, '=');
				Expect(v).ToEqual("===-KiwiOrangeApple!");
				String other{"XX"};
				v.insert(3, other);
				Expect(v).ToEqual("===XX-KiwiOrangeApple!");
				StringView sv{"YY"};
				v.insert(5, sv);
				Expect(v).ToEqual("===XXYY-KiwiOrangeApple!");
				v.insert(0, 2, 'Z');
				Expect(v).ToEqual("ZZ===XXYY-KiwiOrangeApple!");
			});

			It("Can insert with iterator", []()
			{
				String v{"Kiwi"};
				auto it = v.insert(v.begin() + 2, '-');
				Expect(*it).ToEqual('-');
				Expect(v).ToEqual("Ki-wi");
				v.insert(v.end(), 3, '!');
				Expect(v).ToEqual("Ki-wi!!!");
				String other{"AB"};
				v.insert(v.begin(), other.begin(), other.end());
				Expect(v).ToEqual("ABKi-wi!!!");
				v.insert(v.begin() + 2, {'x', 'y'});
				Expect(v).ToEqual("ABxyKi-wi!!!");
			});

			It("Can erase", []()
			{
				String v{"KiwiApple"};
				v.erase(4, 5);
				Expect(v).ToEqual("Kiwi");
				v.erase(2);
				Expect(v).ToEqual("Ki");
				v.erase(0, 1);
				Expect(v).ToEqual("i");
				v.erase(0, 10);
				Expect(v).ToEqual("");
			});

			It("Can erase with iterator", []()
			{
				String v{"Kiwi"};
				auto it = v.erase(v.begin());
				Expect(*it).ToEqual('i');
				Expect(v).ToEqual("iwi");
				v.erase(v.begin() + 1, v.end());
				Expect(v).ToEqual("i");
			});

			It("Can replace", []()
			{
				String v{"KiwiApple"};
				v.replace(0, 4, "Orange");
				Expect(v).ToEqual("OrangeApple");
				v.replace(0, 6, "X");
				Expect(v).ToEqual("XApple");
				v.replace(v.size() - 3, 3, "Z");
				Expect(v).ToEqual("XApZ");
				String other{"Kiwi"};
				v.replace(0, 4, other);
				Expect(v).ToEqual("Kiwi");
				StringView sv{"Two"};
				v.replace(0, 4, sv);
				Expect(v).ToEqual("Two");
				v.replace(0, 3, 2, 'y');
				Expect(v).ToEqual("yy");
			});

			It("Can replace with iterators", []()
			{
				String v{"KiwiApple"};
				v.replace(v.begin(), v.begin() + 4, "Orange");
				Expect(v).ToEqual("OrangeApple");
			});

			It("Can resize", []()
			{
				String v{"Kiwi"};
				v.resize(2);
				Expect(v).ToEqual("Ki");
				v.resize(4);
				Expect(v.size()).ToEqual(4u);
				Expect(v[2]).ToEqual('\0');
				Expect(v[3]).ToEqual('\0');
				v.resize(6, 'x');
				Expect(v[4]).ToEqual('x');
				Expect(v[5]).ToEqual('x');
				Expect(v.size()).ToEqual(6u);
			});

			It("Can swap", []()
			{
				String a{"Kiwi"};
				String b{"Apple"};
				a.swap(b);
				Expect(a).ToEqual("Apple");
				Expect(b).ToEqual("Kiwi");
			});

			It("Can append from self", []()
			{
				String v{longText};
				v.append(v.c_str());
				Expect(v).ToEqual(std::string{longText} + std::string{longText});
			});

			It("Can append self substring", []()
			{
				String v{longText};
				v.append(v.c_str() + 5);
				Expect(v).ToEqual(std::string{longText} + std::string{longText.substr(5)});
			});

			It("Can insert from self", []()
			{
				String v{longText};
				v.insert(0, v.c_str());
				Expect(v).ToEqual(std::string{longText} + std::string{longText});
			});

			It("Can insert self substring", []()
			{
				String v{longText};
				v.insert(4, v.c_str() + 5);
				Expect(v).ToEqual(std::string{longText.substr(0, 4)} + std::string{longText.substr(5)}
				           + std::string{longText.substr(4)});
			});

			It("Can replace with self", []()
			{
				String v{longText};
				v.replace(0, 4, v.c_str());
				Expect(v).ToEqual(std::string{longText} + std::string{longText.substr(4)});
			});

			It("Can replace self substring with count", []()
			{
				String v{longText};
				v.replace(5, 10, v.c_str() + 2, 5);
				Expect(v).ToEqual(std::string{longText.substr(0, 5)} + "23456"
				                     + std::string{longText.substr(15)});
			});
		});

		Describe("Operations", []()
		{
			It("Can get substr", []()
			{
				String v{"KiwiApple"};
				Expect(v.substr()).ToEqual("KiwiApple");
				Expect(v.substr(4)).ToEqual("Apple");
				Expect(v.substr(4, 3)).ToEqual("App");
				Expect(v.substr(0, 100)).ToEqual("KiwiApple");
			});

			It("Can copy out", []()
			{
				String v{"KiwiApple"};
				char buffer[16]{};
				const auto count = v.copy(buffer, 4, 4);
				Expect(count).ToEqual(4u);
				Expect(buffer).ToEqual("Appl");
				buffer[count] = '\0';
			});

			It("Can compare", []()
			{
				String v{"Kiwi"};
				String other{"Kiwi"};
				String apple{"Apple"};
				Expect(v.compare(other)).ToEqual(0);
				Expect(v.compare(apple) > 0).ToBeTrue();
				Expect(apple.compare(v) < 0).ToBeTrue();
				Expect(v.compare("Kiwi")).ToEqual(0);
				Expect(v.compare("Kiwi2") < 0).ToBeTrue();
				Expect(v.compare(StringView{"Kiwi"})).ToEqual(0);
				Expect(v.compare(0, 2, String{"Ki"})).ToEqual(0);
				Expect(v.compare(2, 2, String{"wi"})).ToEqual(0);
			});

			It("Can check prefix and suffix", []()
			{
				String v{"KiwiApple"};
				Expect(v.starts_with("Kiwi")).ToBeTrue();
				Expect(v.starts_with('K')).ToBeTrue();
				Expect(v.starts_with(StringView{"Ki"})).ToBeTrue();
				Expect(v.starts_with("Apple")).ToBeFalse();
				Expect(v.ends_with("Apple")).ToBeTrue();
				Expect(v.ends_with('e')).ToBeTrue();
				Expect(v.ends_with(StringView{"le"})).ToBeTrue();
				Expect(v.ends_with("Kiwi")).ToBeFalse();
			});

			It("Can check contains", []()
			{
				String v{"KiwiApple"};
				Expect(v.contains("wiA")).ToBeTrue();
				Expect(v.contains('A')).ToBeTrue();
				Expect(v.contains(StringView{"zzz"})).ToBeFalse();
				Expect(v.contains('z')).ToBeFalse();
			});

			It("Can find", []()
			{
				String v{"KiwiKiwi"};
				Expect(v.find("Kiwi")).ToEqual(0u);
				Expect(v.find("Kiwi", 1)).ToEqual(4u);
				Expect(v.find("Kiwi", 5)).ToEqual(String::npos);
				Expect(v.find('i')).ToEqual(1u);
				Expect(v.find('i', 6)).ToEqual(7u);
				Expect(v.find('z')).ToEqual(String::npos);
				Expect(v.find(String{"Kiwi"})).ToEqual(0u);
				Expect(v.find(StringView{"Kiwi"})).ToEqual(0u);
			});

			It("Can rfind", []()
			{
				String v{"KiwiKiwi"};
				Expect(v.rfind("Kiwi")).ToEqual(4u);
				Expect(v.rfind("Kiwi", 3)).ToEqual(0u);
				Expect(v.rfind('i')).ToEqual(7u);
				Expect(v.rfind('i', 5)).ToEqual(5u);
				Expect(v.rfind('z')).ToEqual(String::npos);
				Expect(v.rfind(String{"Kiwi"})).ToEqual(4u);
				Expect(v.rfind(StringView{"Kiwi"})).ToEqual(4u);
			});

			It("Can find first of", []()
			{
				String v{"KiwiApple"};
				Expect(v.find_first_of("pl")).ToEqual(5u);
				Expect(v.find_first_of("pl", 6)).ToEqual(6u);
				Expect(v.find_first_of('z')).ToEqual(String::npos);
				Expect(v.find_first_of("xyz")).ToEqual(String::npos);
				Expect(v.find_first_of(StringView{"Ap"})).ToEqual(4u);
			});

			It("Can find last of", []()
			{
				String v{"KiwiApple"};
				Expect(v.find_last_of("pl")).ToEqual(7u);
				Expect(v.find_last_of("pl", 6)).ToEqual(6u);
				Expect(v.find_last_of('z')).ToEqual(String::npos);
				Expect(v.find_last_of(StringView{"Ap"})).ToEqual(6u);
			});

			It("Can find first not of", []()
			{
				String v{"aaab"};
				Expect(v.find_first_not_of("a")).ToEqual(3u);
				Expect(v.find_first_not_of("ab")).ToEqual(String::npos);
				Expect(v.find_first_not_of('a')).ToEqual(3u);
				Expect(v.find_first_not_of("ab", 3)).ToEqual(String::npos);
			});

			It("Can find last not of", []()
			{
				String v{"baaa"};
				Expect(v.find_last_not_of("a")).ToEqual(0u);
				Expect(v.find_last_not_of("ab")).ToEqual(String::npos);
				Expect(v.find_last_not_of('a')).ToEqual(0u);
				Expect(v.find_last_not_of("ab", 0)).ToEqual(String::npos);
			});

			It("Has npos", []()
			{
				Expect(String::npos).ToEqual(sizet(-1));
				Expect(StringView::npos).ToEqual(String::npos);
			});
		});

		Describe("Operators", []()
		{
			It("Can concatenate", []()
			{
				String a{"Kiwi"};
				String b{"Apple"};
				Expect(a + b).ToEqual("KiwiApple");
				Expect(a + "X").ToEqual("KiwiX");
				Expect("X" + a).ToEqual("XKiwi");
				Expect(a + '!').ToEqual("Kiwi!");
				Expect('!' + a).ToEqual("!Kiwi");
				Expect(a + StringView{"V"}).ToEqual("KiwiV");
				Expect(StringView{"V"} + a).ToEqual("VKiwi");
			});

			It("Can chain concatenate", []()
			{
				String a{"Kiwi"};
				String result = a + " " + "Apple" + '!';
				Expect(result).ToEqual("Kiwi Apple!");
			});

			It("Can compare with other types", []()
			{
				String v{"Kiwi"};
				Expect(v == String{"Kiwi"}).ToBeTrue();
				Expect(v != String{"Apple"}).ToBeTrue();
				Expect(v == "Kiwi").ToBeTrue();
				Expect(v != "Apple").ToBeTrue();
				Expect("Kiwi" == v).ToBeTrue();
				Expect("Apple" != v).ToBeTrue();
				Expect(v < "Lime").ToBeTrue();
				Expect("Lime" > v).ToBeTrue();
				Expect(v <= String{"Kiwi"}).ToBeTrue();
				Expect(v >= String{"Kiwi"}).ToBeTrue();
				Expect(v == StringView{"Kiwi"}).ToBeTrue();
				Expect(StringView{"Kiwi"} == v).ToBeTrue();
				Expect(v != StringView{"Apple"}).ToBeTrue();
				Expect(StringView{"Apple"} != v).ToBeTrue();
				Expect(v < StringView{"Lime"}).ToBeTrue();
				Expect(StringView{"Lime"} > v).ToBeTrue();
			});

			It("Can three-way compare", []()
			{
				String a{"Kiwi"};
				String b{"Lime"};
				Expect((a <=> b) < 0).ToBeTrue();
				Expect((b <=> a) > 0).ToBeTrue();
				Expect((a <=> String{"Kiwi"}) == 0).ToBeTrue();
				Expect((a <=> "Kiwi") == 0).ToBeTrue();
			});
		});

		Describe("Memory", []()
		{
			It("Keeps data valid when growing", []()
			{
				String v;
				for (char c = 'a'; c <= 'z'; ++c)
				{
					v.push_back(c);
				}
				Expect(v.size()).ToEqual(26u);
				Expect(v).ToEqual("abcdefghijklmnopqrstuvwxyz");
				Expect(v.c_str()[26]).ToEqual('\0');
			});

			It("Can reuse capacity", []()
			{
				String v;
				v.reserve(1000);
				const auto cap = v.capacity();
				for (u32 i = 0; i < 100; ++i)
				{
					v.assign("KiwiAppleOrangeBanana");
					v.clear();
				}
				Expect(v.capacity()).ToEqual(cap);
			});

			It("Is valid after move assignment", []()
			{
				String a{"Kiwi"};
				String b;
				b = Move(a);
				Expect(b).ToEqual("Kiwi");
				a = "Reused";
				Expect(a).ToEqual("Reused");
			});
		});

		Describe("Format & Hash", []()
		{
			It("Can be formatted", []()
			{
				String v{"Kiwi"};
				Expect(std::format("{}", v)).ToEqual("Kiwi");
				Expect(Format("{}-{}", v, 5)).ToEqual("Kiwi-5");
				String out;
				FormatTo(out, "{}!", v);
				Expect(out).ToEqual("Kiwi!");
			});

			It("Can be hashed", []()
			{
				String v{"Kiwi"};
				Expect(GetHash(v)).ToEqual(GetStringHash("Kiwi"));
				Expect(GetHash(StringView{"Kiwi"})).ToEqual(GetHash(v));
			});
		});

		Describe("Arena", []()
		{
			It("Can default construct on an arena", []()
			{
				MonoLinearArena arena{Memory::KB * 4};
				String v{arena};
				Expect(v.empty()).ToBeTrue();
				Expect(&v.GetArena()).ToEqual(static_cast<Arena*>(&arena));
				// Short strings still use the inline buffer
				v = "Kiwi";
				Expect(v).ToEqual("Kiwi");
				Expect(v.capacity() <= 32u).ToBeTrue();
			});

			It("Can allocate on an arena", []()
			{
				MonoLinearArena arena{Memory::KB * 4};
				String v{arena, arenaLongText};
				Expect(v).ToEqual(arenaLongText);
				Expect(&v.GetArena()).ToEqual(static_cast<Arena*>(&arena));
				// Long strings must allocate on the arena, not the current arena
				Expect(v.capacity() >= v.size()).ToBeTrue();
			});

			It("Can construct with count and char on an arena", []()
			{
				MonoLinearArena arena{Memory::KB * 4};
				String v{arena, 64, 'x'};
				Expect(v.size()).ToEqual(64u);
				Expect(&v.GetArena()).ToEqual(static_cast<Arena*>(&arena));
			});

			It("Can copy into an arena", []()
			{
				MonoLinearArena arena{Memory::KB * 4};
				String original{arenaLongText};
				String v{arena, original};
				Expect(v).ToEqual(original);
				Expect(&v.GetArena()).ToEqual(static_cast<Arena*>(&arena));
			});

			It("Keeps its arena when assigned", []()
			{
				MonoLinearArena arena{Memory::KB * 4};
				String v{arena};
				v.assign(arenaLongText);
				v.append(" with some extra content to force a reallocation");
				Expect(&v.GetArena()).ToEqual(static_cast<Arena*>(&arena));
				Expect(v.starts_with("This string")).ToBeTrue();
			});
		});

		Describe("Strings helpers", []()
		{
			It("RemoveFromStart", []()
			{
				String v{"KiwiApple"};
				Strings::RemoveFromStart(v, 4);
				Expect(v).ToEqual("Apple");
				Strings::RemoveFromStart(v, 100);
				Expect(v.empty()).ToBeTrue();
			});

			It("RemoveFromEnd", []()
			{
				String v{"KiwiApple"};
				Strings::RemoveFromEnd(v, 5);
				Expect(v).ToEqual("Kiwi");
				Strings::RemoveFromEnd(v, StringView{"wi"});
				Expect(v).ToEqual("Ki");
				Strings::RemoveFromEnd(v, 100);
				Expect(v.empty()).ToBeTrue();
			});

			It("RemoveCharFromEnd", []()
			{
				String v{"Kiwi!"};
				Expect(Strings::RemoveCharFromEnd(v, '!')).ToBeTrue();
				Expect(v).ToEqual("Kiwi");
				Expect(Strings::RemoveCharFromEnd(v, '!')).ToBeFalse();
				Expect(v).ToEqual("Kiwi");
			});

			It("ToSentenceCase", []()
			{
				Expect(Strings::ToSentenceCase("")).ToEqual("");
				Expect(Strings::ToSentenceCase("papa")).ToEqual("Papa");
				Expect(Strings::ToSentenceCase("papa ")).ToEqual("Papa ");
				Expect(Strings::ToSentenceCase("papa3")).ToEqual("Papa 3");
				Expect(Strings::ToSentenceCase("MisterPotato")).ToEqual("Mister Potato");
			});

			It("Convert u16 to u8", []()
			{
				TString<Char16> utf16string{0x41, 0x0448, 0x65e5, 0xd834, 0xdd1e};
				TString<Char8> u = Strings::Convert<TString<Char8>>(utf16string);
				Expect(u.size()).ToEqual(10u);
			});
			It("Convert u8 to u16", []()
			{
				TString<AnsiChar> utf8_with_surrogates = "\xe6\x97\xa5\xd1\x88\xf0\x9d\x84\x9e";
				TString<Char16> utf16result =
				    Strings::Convert<TString<Char16>>(utf8_with_surrogates);
				Expect(utf16result.size()).ToEqual(4u);
				Expect(utf16result[2] == 0xd834).ToBeTrue();
				Expect(utf16result[3] == 0xdd1e).ToBeTrue();
			});
			It("Convert u32 to u8", []()
			{
				TString<Char32> utf32string = {0x448, 0x65E5, 0x10346};
				TString<Char8> utf8result   = Strings::Convert<TString<Char8>>(utf32string);
				Expect(utf8result.size()).ToEqual(9u);
			});
			It("Convert u8 to u32", []()
			{
				TString<AnsiChar> twochars  = "\xe6\x97\xa5\xd1\x88";
				TString<Char32> utf32result = Strings::Convert<TString<Char32>>(twochars);
				Expect(utf32result.size()).ToEqual(2u);
			});
		});
	});
});
return true;
}();
}    // namespace
