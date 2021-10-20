// Copyright 2015-2021 Piperift - All rights reserved

#include <Math/Color.h>
#include <bandit/bandit.h>

#include <cmath>
#include <limits>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;


namespace snowhouse
{
	template <>
	struct Stringizer<u8>
	{
		static std::string ToString(u8 a)
		{
			std::stringstream stream;
			stream << u32(a);
			return stream.str();
		}
	};
	template <>
	struct Stringizer<Color>
	{
		static std::string ToString(const Color& a)
		{
			std::stringstream stream;
			stream << "Color(" << u32(a.r) << ", " << u32(a.g) << ", " << u32(a.b) << ", "
			       << u32(a.a) << ")";
			return stream.str();
		}
	};

	template <>
	struct Stringizer<LinearColor>
	{
		static std::string ToString(const LinearColor& a)
		{
			std::stringstream stream;
			stream << "LinearColor(" << a.r << ", " << a.g << ", " << a.b << ", " << a.a << ")";
			return stream.str();
		}
	};
}    // namespace snowhouse


go_bandit([]() {
	describe("Math.Color", []() {
		describe("LinearColor", []() {
			it("Can darken", [&]() {
				AssertThat(LinearColor::White.Darken(1.0f), Equals(LinearColor::Black));
				AssertThat(LinearColor::White.Darken(0.5f), Equals(LinearColor::Gray));
				constexpr LinearColor color{Color::HexRGB(0x80ced7)};
				AssertThat(color.Darken(0.5f), Equals(LinearColor{Color::HexRGB(0x40676B)}));
			});

			it("Darken doesn't change alpha", [&]() {
				AssertThat(LinearColor::White.Translucency(0.5f).Darken(1.0f).a,
				    EqualsWithDelta(0.5f, 0.01f));
			});

			it("Can lighten", [&]() {
				AssertThat(LinearColor::Black.Lighten(1.0f), Equals(LinearColor::White));
				AssertThat(LinearColor::Black.Lighten(0.5f), Equals(LinearColor::Gray));
				AssertThat(Color::HexRGB(0x80ced7).Lighten(0.5f), Equals(Color::HexRGB(0xbfe6eb)));
			});

			it("Lighten doesn't change alpha", [&]() {
				AssertThat(LinearColor::Black.Translucency(0.5f).Lighten(1.0f).a,
				    EqualsWithDelta(0.5f, 0.01f));
			});
		});
		describe("Color", []() {
			it("Can darken", [&]() {
				AssertThat(Color::White.Darken(1.0f), Equals(Color::Black));
				AssertThat(Color::White.Darken(0.5f), Equals(Color::Gray));
				AssertThat(Color::HexRGB(0x80ced7).Darken(0.5f), Equals(Color::HexRGB(0x40676B)));
			});

			it("Darken doesn't change alpha", [&]() {
				AssertThat(Color::White.Translucency(127).Darken(1.0f).a, Equals(127));
			});

			it("Can lighten", [&]() {
				AssertThat(Color::Black.Lighten(1.0f), Equals(Color::White));
				AssertThat(Color::Black.Lighten(0.5f), Equals(Color::Gray));
				AssertThat(Color::HexRGB(0x80ced7).Lighten(0.5f), Equals(Color::HexRGB(0xbfe6eb)));
			});

			it("Lighten doesn't change alpha", [&]() {
				AssertThat(Color::Black.Translucency(127).Lighten(1.0f).a, Equals(127));
			});

			it("Can convert to linear", [&]() {
				AssertThat(LinearColor{Color::White}, Equals(LinearColor::White));
				AssertThat(LinearColor{Color::Black}, Equals(LinearColor::Black));
				AssertThat(LinearColor{Color::Gray}, Equals(LinearColor::Gray));
			});
		});
	});
});
