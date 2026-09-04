// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <PipeColor.h>


using namespace p;


Spec("Math.Color", []()
{
Describe("Helpers", []()
{
	It("Can make from rgba", []()
	{
		auto color = Color::FromRGB(128, 206, 215, 35);
		Expect(color.r).ToEqual(128);
		Expect(color.g).ToEqual(206);
		Expect(color.b).ToEqual(215);
		Expect(color.a).ToEqual(35);
	});
	It("Can make from Hex", []()
	{
		auto color = Color::FromHex(0x80ced7);
		Expect(color.r).ToEqual(128);
		Expect(color.g).ToEqual(206);
		Expect(color.b).ToEqual(215);

		auto colora = Color::FromHexAlpha(0x80ced723);
		Expect(colora.r).ToEqual(128);
		Expect(colora.g).ToEqual(206);
		Expect(colora.b).ToEqual(215);
		Expect(colora.a).ToEqual(35);
	});

	It("Can make from packed", []()
	{
		auto argb = Color::FromPackedARGB(0x2380ced7);
		Expect(argb.r).ToEqual(128);
		Expect(argb.g).ToEqual(206);
		Expect(argb.b).ToEqual(215);
		Expect(argb.a).ToEqual(35);

		auto abgr = Color::FromPackedABGR(0x23d7ce80);
		Expect(abgr.r).ToEqual(128);
		Expect(abgr.g).ToEqual(206);
		Expect(abgr.b).ToEqual(215);
		Expect(abgr.a).ToEqual(35);

		auto rgba = Color::FromPackedRGBA(0x80ced723);
		Expect(rgba.r).ToEqual(128);
		Expect(rgba.g).ToEqual(206);
		Expect(rgba.b).ToEqual(215);
		Expect(rgba.a).ToEqual(35);

		auto bgra = Color::FromPackedBGRA(0xd7ce8023);
		Expect(bgra.r).ToEqual(128);
		Expect(bgra.g).ToEqual(206);
		Expect(bgra.b).ToEqual(215);
		Expect(bgra.a).ToEqual(35);
	});

	It("Can get as packed", []()
	{
		auto color = Color(128, 206, 215, 35);
		Expect(color.ToPackedARGB()).ToEqual(0x2380ced7);
		Expect(color.ToPackedABGR()).ToEqual(0x23d7ce80);
		Expect(color.ToPackedRGBA()).ToEqual(0x80ced723);
		Expect(color.ToPackedBGRA()).ToEqual(0xd7ce8023);
	});
});
Describe("LinearColor", []()
{
	It("Can Shade", []()
	{
		Expect(LinearColor::White().Shade(1.0f)).ToEqual(LinearColor::Black());
		Expect(LinearColor::White().Shade(0.5f)).ToEqual(LinearColor::Gray());
		constexpr LinearColor color{Color::FromHex(0x80ced7)};
		Expect(color.Shade(0.5f)).ToEqual(LinearColor{Color::FromHex(0x40676B)});
	});

	It("Shade doesn't change alpha", []()
	{
		Expect(std::abs(LinearColor::White().Translucency(0.5f).Shade(1.0f).a - 0.5f))
		    .ToBeLessOrEqual(0.01f);
	});

	It("Can Tint", []()
	{
		Expect(LinearColor::Black().Tint(1.0f)).ToEqual(LinearColor::White());
		Expect(LinearColor::Black().Tint(0.5f)).ToEqual(LinearColor::Gray());
		Expect(Color::FromHex(0x80ced7).Tint(0.5f)).ToEqual(Color::FromHex(0xbfe6eb));
	});

	It("Tint doesn't change alpha", []()
	{
		Expect(std::abs(LinearColor::Black().Translucency(0.5f).Tint(1.0f).a - 0.5f))
		    .ToBeLessOrEqual(0.01f);
	});
});
Describe("Color", []()
{
	It("Can Shade", []()
	{
		Expect(Color::White().Shade(1.0f)).ToEqual(Color::Black());
		Expect(Color::White().Shade(0.5f)).ToEqual(Color::Gray());
		Expect(Color::FromHex(0x80ced7).Shade(0.5f)).ToEqual(Color::FromHex(0x40676B));
	});

	It("Shade doesn't change alpha", []()
	{
		Expect(Color::White().Translucency(127).Shade(1.0f).a).ToEqual(127);
	});

	It("Can Tint", []()
	{
		Expect(Color::Black().Tint(1.0f)).ToEqual(Color::White());
		Expect(Color::Black().Tint(0.5f)).ToEqual(Color::Gray());
		Expect(Color::FromHex(0x80ced7).Tint(0.5f)).ToEqual(Color::FromHex(0xbfe6eb));
	});

	It("Tint doesn't change alpha", []()
	{
		Expect(Color::Black().Translucency(127).Tint(1.0f).a).ToEqual(127);
	});

	It("Can convert to linear", []()
	{
		Expect(LinearColor{Color::White()}).ToEqual(LinearColor::White());
		Expect(LinearColor{Color::Black()}).ToEqual(LinearColor::Black());
		Expect(LinearColor{Color::Gray()}).ToEqual(LinearColor::Gray());
	});
});
});
