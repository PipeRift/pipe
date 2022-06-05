// Copyright 2015-2022 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Math/Color.h>

#include <cmath>
#include <limits>


using namespace snowhouse;
using namespace bandit;
using namespace pipe;


namespace snowhouse
{
	template<>
	struct Stringizer<u8>
	{
		static std::string ToString(u8 a)
		{
			std::stringstream stream;
			stream << u32(a);
			return stream.str();
		}
	};
	template<>
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

	template<>
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
			it("Can Shade", [&]() {
				AssertThat(LinearColor::White().Shade(1.0f), Equals(LinearColor::Black()));
				AssertThat(LinearColor::White().Shade(0.5f), Equals(LinearColor::Gray()));
				constexpr LinearColor color{Color::FromHEX(0x80ced7)};
				AssertThat(color.Shade(0.5f), Equals(LinearColor{Color::FromHEX(0x40676B)}));
			});

			it("Shade doesn't change alpha", [&]() {
				AssertThat(LinearColor::White().Translucency(0.5f).Shade(1.0f).a,
				    EqualsWithDelta(0.5f, 0.01f));
			});

			it("Can Tint", [&]() {
				AssertThat(LinearColor::Black().Tint(1.0f), Equals(LinearColor::White()));
				AssertThat(LinearColor::Black().Tint(0.5f), Equals(LinearColor::Gray()));
				AssertThat(Color::FromHEX(0x80ced7).Tint(0.5f), Equals(Color::FromHEX(0xbfe6eb)));
			});

			it("Tint doesn't change alpha", [&]() {
				AssertThat(LinearColor::Black().Translucency(0.5f).Tint(1.0f).a,
				    EqualsWithDelta(0.5f, 0.01f));
			});
		});
		describe("Color", []() {
			it("Can Shade", [&]() {
				AssertThat(Color::White().Shade(1.0f), Equals(Color::Black()));
				AssertThat(Color::White().Shade(0.5f), Equals(Color::Gray()));
				AssertThat(Color::FromHEX(0x80ced7).Shade(0.5f), Equals(Color::FromHEX(0x40676B)));
			});

			it("Shade doesn't change alpha", [&]() {
				AssertThat(Color::White().Translucency(127).Shade(1.0f).a, Equals(127));
			});

			it("Can Tint", [&]() {
				AssertThat(Color::Black().Tint(1.0f), Equals(Color::White()));
				AssertThat(Color::Black().Tint(0.5f), Equals(Color::Gray()));
				AssertThat(Color::FromHEX(0x80ced7).Tint(0.5f), Equals(Color::FromHEX(0xbfe6eb)));
			});

			it("Tint doesn't change alpha", [&]() {
				AssertThat(Color::Black().Translucency(127).Tint(1.0f).a, Equals(127));
			});

			it("Can convert to linear", [&]() {
				AssertThat(LinearColor{Color::White()}, Equals(LinearColor::White()));
				AssertThat(LinearColor{Color::Black()}, Equals(LinearColor::Black()));
				AssertThat(LinearColor{Color::Gray()}, Equals(LinearColor::Gray()));
			});
		});
	});
});
