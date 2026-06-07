// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <bandit/bandit.h>
#include <PipeColor.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;


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


go_bandit([]()
{
	describe("Math.Color", [&]()
	{
		describe("Helpers", [&]()
		{
			it("Can make from rgba", [&]()
			{
				auto color = Color::FromRGB(128, 206, 215, 35);
				AssertThat(color.r, Equals(128));
				AssertThat(color.g, Equals(206));
				AssertThat(color.b, Equals(215));
				AssertThat(color.a, Equals(35));
			});
			it("Can make from Hex", [&]()
			{
				auto color = Color::FromHex(0x80ced7);
				AssertThat(color.r, Equals(128));
				AssertThat(color.g, Equals(206));
				AssertThat(color.b, Equals(215));

				auto colora = Color::FromHexAlpha(0x80ced723);
				AssertThat(colora.r, Equals(128));
				AssertThat(colora.g, Equals(206));
				AssertThat(colora.b, Equals(215));
				AssertThat(colora.a, Equals(35));
			});

			it("Can make from packed", [&]()
			{
				auto argb = Color::FromPackedARGB(0x2380ced7);
				AssertThat(argb.r, Equals(128));
				AssertThat(argb.g, Equals(206));
				AssertThat(argb.b, Equals(215));
				AssertThat(argb.a, Equals(35));

				auto abgr = Color::FromPackedABGR(0x23d7ce80);
				AssertThat(abgr.r, Equals(128));
				AssertThat(abgr.g, Equals(206));
				AssertThat(abgr.b, Equals(215));
				AssertThat(abgr.a, Equals(35));

				auto rgba = Color::FromPackedRGBA(0x80ced723);
				AssertThat(rgba.r, Equals(128));
				AssertThat(rgba.g, Equals(206));
				AssertThat(rgba.b, Equals(215));
				AssertThat(rgba.a, Equals(35));

				auto bgra = Color::FromPackedBGRA(0xd7ce8023);
				AssertThat(bgra.r, Equals(128));
				AssertThat(bgra.g, Equals(206));
				AssertThat(bgra.b, Equals(215));
				AssertThat(bgra.a, Equals(35));
			});

			it("Can get as packed", [&]()
			{
				auto color = Color(128, 206, 215, 35);
				AssertThat(color.ToPackedARGB(), Equals(0x2380ced7));
				AssertThat(color.ToPackedABGR(), Equals(0x23d7ce80));
				AssertThat(color.ToPackedRGBA(), Equals(0x80ced723));
				AssertThat(color.ToPackedBGRA(), Equals(0xd7ce8023));
			});
		});
		describe("LinearColor", [&]()
		{
			it("Can Shade", [&]()
			{
				AssertThat(LinearColor::White().Shade(1.0f), Equals(LinearColor::Black()));
				AssertThat(LinearColor::White().Shade(0.5f), Equals(LinearColor::Gray()));
				constexpr LinearColor color{Color::FromHex(0x80ced7)};
				AssertThat(color.Shade(0.5f), Equals(LinearColor{Color::FromHex(0x40676B)}));
			});

			it("Shade doesn't change alpha", [&]()
			{
				AssertThat(LinearColor::White().Translucency(0.5f).Shade(1.0f).a,
				    EqualsWithDelta(0.5f, 0.01f));
			});

			it("Can Tint", [&]()
			{
				AssertThat(LinearColor::Black().Tint(1.0f), Equals(LinearColor::White()));
				AssertThat(LinearColor::Black().Tint(0.5f), Equals(LinearColor::Gray()));
				AssertThat(Color::FromHex(0x80ced7).Tint(0.5f), Equals(Color::FromHex(0xbfe6eb)));
			});

			it("Tint doesn't change alpha", [&]()
			{
				AssertThat(LinearColor::Black().Translucency(0.5f).Tint(1.0f).a,
				    EqualsWithDelta(0.5f, 0.01f));
			});
		});
		describe("Color", [&]()
		{
			it("Can Shade", [&]()
			{
				AssertThat(Color::White().Shade(1.0f), Equals(Color::Black()));
				AssertThat(Color::White().Shade(0.5f), Equals(Color::Gray()));
				AssertThat(Color::FromHex(0x80ced7).Shade(0.5f), Equals(Color::FromHex(0x40676B)));
			});

			it("Shade doesn't change alpha", [&]()
			{
				AssertThat(Color::White().Translucency(127).Shade(1.0f).a, Equals(127));
			});

			it("Can Tint", [&]()
			{
				AssertThat(Color::Black().Tint(1.0f), Equals(Color::White()));
				AssertThat(Color::Black().Tint(0.5f), Equals(Color::Gray()));
				AssertThat(Color::FromHex(0x80ced7).Tint(0.5f), Equals(Color::FromHex(0xbfe6eb)));
			});

			it("Tint doesn't change alpha", [&]()
			{
				AssertThat(Color::Black().Translucency(127).Tint(1.0f).a, Equals(127));
			});

			it("Can convert to linear", [&]()
			{
				AssertThat(LinearColor{Color::White()}, Equals(LinearColor::White()));
				AssertThat(LinearColor{Color::Black()}, Equals(LinearColor::Black()));
				AssertThat(LinearColor{Color::Gray()}, Equals(LinearColor::Gray()));
			});
		});
	});
});
