// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
    Color.cpp: Unreal color implementation.
=============================================================================*/

#include "Math/Color.h"

#include "Math/Vector.h"
#include "Serialization/Contexts.h"


namespace Rift
{
	// Common colors.
	const LinearColor LinearColor::White{1.f, 1.f, 1.f};
	const LinearColor LinearColor::Gray{Color::Gray};
	const LinearColor LinearColor::Black{0.f, 0.f, 0.f};
	const LinearColor LinearColor::Transparent{0.f, 0.f, 0.f, 0.f};
	const LinearColor LinearColor::Red{1.f, 0.f, 0.f};
	const LinearColor LinearColor::Green{0.f, 1.f, 0.f};
	const LinearColor LinearColor::Blue{0.f, 0.f, 1.f};
	const LinearColor LinearColor::Yellow{1.f, 1.f, 0.f};

	const Color Color::White{255, 255, 255};
	const Color Color::Gray{127, 127, 127};
	const Color Color::Black{0, 0, 0};
	const Color Color::Transparent{0, 0, 0, 0};
	const Color Color::Red{255, 0, 0};
	const Color Color::Green{0, 255, 0};
	const Color Color::Blue{0, 0, 255};
	const Color Color::Yellow{255, 255, 0};
	const Color Color::Cyan{0, 255, 255};
	const Color Color::Magenta{255, 0, 255};
	const Color Color::Orange{243, 156, 18};
	const Color Color::Purple{169, 7, 228};
	const Color Color::Turquoise{26, 188, 156};
	const Color Color::Silver{189, 195, 199};
	const Color Color::Emerald{46, 204, 113};


	LinearColor LinearColor::FromSRGBColor(const Color& color)
	{
		LinearColor lColor;
		lColor.r = float(sRGBToLinearTable[color.r]);
		lColor.g = float(sRGBToLinearTable[color.g]);
		lColor.b = float(sRGBToLinearTable[color.b]);
		lColor.a = float(color.a) * OneOver255;

		return lColor;
	}

	LinearColor LinearColor::FromPow22Color(const Color& color)
	{
		LinearColor linearColor;
		linearColor.r = float(Pow22OneOver255Table[color.r]);
		linearColor.g = float(Pow22OneOver255Table[color.g]);
		linearColor.b = float(Pow22OneOver255Table[color.b]);
		linearColor.a = float(color.a) * OneOver255;

		return linearColor;
	}

	// Convert from float to RGBE as outlined in Gregory Ward's Real Pixels article, Graphics Gems
	// II, page 80.
	Color LinearColor::ToRGBE() const
	{
		const float Primary = Math::Max(r, g, b);
		Color color;

		if (Primary < 1E-32)
		{
			color = Color(0, 0, 0, 0);
		}
		else
		{
			i32 exponent;
			const float scale = frexp(Primary, &exponent) / Primary * 255.f;

			color.r = (u8) Math::Clamp(Math::FloorToI32(r * scale), 0, 255);
			color.g = (u8) Math::Clamp(Math::FloorToI32(g * scale), 0, 255);
			color.b = (u8) Math::Clamp(Math::FloorToI32(b * scale), 0, 255);
			color.a = (u8)(Math::Clamp(exponent, -128, 127) + 128);
		}

		return color;
	}


	Color LinearColor::Quantize() const
	{
		return Color((u8) Math::Clamp<i32>(Math::FloorToI32(r * 255.f), 0, 255),
		    (u8) Math::Clamp<i32>(Math::FloorToI32(g * 255.f), 0, 255),
		    (u8) Math::Clamp<i32>(Math::FloorToI32(b * 255.f), 0, 255),
		    (u8) Math::Clamp<i32>(Math::FloorToI32(a * 255.f), 0, 255));
	}

	Color LinearColor::QuantizeRound() const
	{
		return Color((u8) Math::Clamp<i32>(Math::RoundToInt(r * 255.f), 0, 255),
		    (u8) Math::Clamp<i32>(Math::RoundToInt(g * 255.f), 0, 255),
		    (u8) Math::Clamp<i32>(Math::RoundToInt(b * 255.f), 0, 255),
		    (u8) Math::Clamp<i32>(Math::RoundToInt(a * 255.f), 0, 255));
	}

	/**
	 * Returns a desaturated color, with 0 meaning no desaturation and 1 == full desaturation
	 *
	 * @param	desaturation factor in range [0..1]
	 * @return	Desaturated color
	 */
	LinearColor LinearColor::Desaturate(float desaturation) const
	{
		float Lum = ComputeLuminance();
		return Math::Lerp(*this, LinearColor(Lum, Lum, Lum, 0), desaturation);
	}

	LinearColor LinearColor::Darken(float delta) const
	{
		return {ToColor(true).Darken(delta)};
	}

	LinearColor LinearColor::Lighten(float delta) const
	{
		return {ToColor(true).Lighten(delta)};
	}

	// Convert from RGBE to float as outlined in Gregory Ward's Real Pixels article, Graphics Gems
	// II, page 80.
	LinearColor Color::FromRGBE() const
	{
		if (a == 0)
			return LinearColor::Black;
		else
		{
			const float scale = (float) ldexp(1 / 255.0, a - 128);
			return LinearColor(r * scale, g * scale, b * scale, 1.0f);
		}
	}

	/**
	 * Converts byte hue-saturation-brightness to floating point red-green-blue.
	 */
	LinearColor LinearColor::GetHSV(u8 h, u8 s, u8 v)
	{
		float brightness = v * 1.4f / 255.f;
		brightness *= 0.7f / (0.01f + Math::Sqrt(brightness));
		brightness = Math::Clamp(brightness, 0.f, 1.f);

		v3 hue;
		if (h < 86)
		{
			hue = v3((85.f - h) / 85.f, float(h) / 85.f, 0.f);
		}
		else if (h < 171)
		{
			hue = v3(0.f, (170.f - h) / 85.f, (float(h) - 85.f) / 85.f);
		}
		else
		{
			hue = v3((float(h) - 170.f) / 85.f, 0.f, (255.f - h) / 84.f);
		}

		const v3 colorVector = (hue + (v3::One() - hue) * (s / 255.f)) * brightness;
		return LinearColor(colorVector.x, colorVector.y, colorVector.z, 1);
	}


	/**
	 * Makes a random but quite nice color.
	 */
	LinearColor LinearColor::MakeRandomColor()
	{
		const u8 Hue = (u8)(Math::Rand01() * 255.f);
		return LinearColor::GetHSV(Hue, 0, 255);
	}

	Color Color::MakeRandomColor()
	{
		return LinearColor::MakeRandomColor().ToColor(true);
	}

	LinearColor LinearColor::MakeFromColorTemperature(float Temp)
	{
		Temp = Math::Clamp(Temp, 1000.0f, 15000.0f);

		// Approximate Planckian locus in CIE 1960 UCS
		float u = (0.860117757f + 1.54118254e-4f * Temp + 1.28641212e-7f * Temp * Temp) /
		          (1.0f + 8.42420235e-4f * Temp + 7.08145163e-7f * Temp * Temp);
		float v = (0.317398726f + 4.22806245e-5f * Temp + 4.20481691e-8f * Temp * Temp) /
		          (1.0f - 2.89741816e-5f * Temp + 1.61456053e-7f * Temp * Temp);

		float x = 3.0f * u / (2.0f * u - 8.0f * v + 4.0f);
		float y = 2.0f * v / (2.0f * u - 8.0f * v + 4.0f);
		float z = 1.0f - x - y;

		float Y = 1.0f;
		float X = Y / y * x;
		float Z = Y / y * z;

		// XYZ to RGB with BT.709 primaries
		float r = 3.2404542f * X + -1.5371385f * Y + -0.4985314f * Z;
		float g = -0.9692660f * X + 1.8760108f * Y + 0.0415560f * Z;
		float b = 0.0556434f * X + -0.2040259f * Y + 1.0572252f * Z;

		return LinearColor(r, g, b);
	}

	Color Color::MakeFromColorTemperature(float Temp)
	{
		return LinearColor::MakeFromColorTemperature(Temp).ToColor(true);
	}

	Color Color::MakeRedToGreenColorFromScalar(float scalar)
	{
		float redSclr   = Math::Clamp<float>((1.0f - scalar) / 0.5f, 0.f, 1.f);
		float greenSclr = Math::Clamp<float>((scalar / 0.5f), 0.f, 1.f);

		u8 r = (u8) Math::FloorToI32(255 * redSclr);
		u8 g = (u8) Math::FloorToI32(255 * greenSclr);
		u8 b = 0;
		return Color(r, g, b);
	}

	Color Color::Desaturate(float desaturation) const
	{
		LinearColor self{*this};
		self.Desaturate(desaturation);
		return self.ToColor(false);
	}

	void ComputeAndFixedColorAndIntensity(
	    const LinearColor& InLinearColor, Color& OutColor, float& OutIntensity)
	{
		float MaxComponent =
		    Math::Max(Math::SMALLER_NUMBER, InLinearColor.r, InLinearColor.g, InLinearColor.b);
		OutColor     = (InLinearColor / MaxComponent).ToColor(true);
		OutIntensity = MaxComponent;
	}

	void Read(Serl::ReadContext& ct, LinearColor& color)
	{
		ct.BeginObject();
		ct.Next("r", color.r);
		ct.Next("g", color.g);
		ct.Next("b", color.b);
		ct.Next("a", color.a);
	}

	void Write(Serl::WriteContext& ct, const LinearColor& color)
	{
		ct.BeginObject();
		ct.Next("r", color.r);
		ct.Next("g", color.g);
		ct.Next("b", color.b);
		ct.Next("a", color.a);
	}

	void Read(Serl::ReadContext& ct, Color& color)
	{
		ct.BeginObject();
		ct.Next("r", color.r);
		ct.Next("g", color.g);
		ct.Next("b", color.b);
		ct.Next("a", color.a);
	}

	void Write(Serl::WriteContext& ct, Color color)
	{
		ct.BeginObject();
		ct.Next("r", color.r);
		ct.Next("g", color.g);
		ct.Next("b", color.b);
		ct.Next("a", color.a);
	}
}    // namespace Rift
