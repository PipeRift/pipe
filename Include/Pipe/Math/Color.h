// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Math.h"
#include "Pipe/Core/Array.h"
#include "Pipe/Core/FixedString.h"
#include "Pipe/Core/String.h"
#include "Pipe/Reflect/NativeType.h"
#include "Pipe/Reflect/TypeFlags.h"
#include "Pipe/Serialize/Serialization.h"
#include "Vector.h"


namespace p
{
	/**
	 * Different color space representations supported
	 */
	enum class ColorMode : u8
	{
		RGBA,      // 8-bit/component integral RGBA color
		Linear,    // 32-bit/component. A linear floating point RGBA color with no gamma correction
		sRGB,      // 32-bit/component. Standard sRGB floating point RGBA color
		HSV        // 32-bit/component. Hue-Saturation-Value floating point RGBA color
	};


	template<ColorMode mode>
	struct TColorData
	{
		float r, g, b, a;

	protected:
		constexpr TColorData() : r(0.f), g(0.f), b(0.f), a(1.f) {}
		constexpr TColorData(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
	};


	template<>
	struct TColorData<ColorMode::RGBA>
	{
#pragma warning(disable:4201)    // Avoid warning about nameless struct
		union
		{
			struct
			{
				u8 r, g, b, a;
			};
			u32 alignmentDummy;
		};
#pragma warning(default:4201)

	protected:
		constexpr TColorData() : r(0), g(0), b(0), a(255){};
		constexpr TColorData(u8 r, u8 g, u8 b, u8 a) : r(r), g(g), b(b), a(a) {}
	};

	template<>
	struct TColorData<ColorMode::HSV>
	{
		float h, s, v, a;

	protected:
		constexpr TColorData() : h(0.f), s(0.f), v(0.f), a(1.f) {}
		constexpr TColorData(float h, float s, float v, float a) : h(h), s(s), v(v), a(a) {}
	};


	template<ColorMode Mode>
	struct TColor : public TColorData<Mode>
	{
		static constexpr ColorMode mode = Mode;

	private:
		static constexpr float oneOver255 = 1.0f / 255.0f;

		// clang-format off
		/**
		 * Table for fast Color -> FTColor conversion.
		 *
		 * Color > 0.04045 ? pow( Color * (1.0 / 1.055) + 0.0521327, 2.4 ) : Color * (1.0 / 12.92);
		 */
		static constexpr double sRGBToLinearTable[256] {
		    0, 0.000303526983548838, 0.000607053967097675, 0.000910580950646512, 0.00121410793419535, 0.00151763491774419, 0.00182116190129302, 0.00212468888484186, 0.0024282158683907, 0.00273174285193954, 0.00303526983548838, 0.00334653564113713, 0.00367650719436314, 0.00402471688178252, 0.00439144189356217, 0.00477695332960869, 0.005181516543916, 0.00560539145834456, 0.00604883284946662, 0.00651209061157708, 0.00699540999852809, 0.00749903184667767, 0.00802319278093555, 0.0085681254056307, 0.00913405848170623, 0.00972121709156193, 0.0103298227927056, 0.0109600937612386, 0.0116122449260844, 0.012286488094766, 0.0129830320714536, 0.0137020827679224, 0.0144438433080002, 0.0152085141260192, 0.0159962930597398, 0.0168073754381669, 0.0176419541646397, 0.0185002197955389, 0.0193823606149269, 0.0202885627054049, 0.0212190100154473, 0.0221738844234532, 0.02315336579873, 0.0241576320596103, 0.0251868592288862, 0.0262412214867272, 0.0273208912212394, 0.0284260390768075, 0.0295568340003534, 0.0307134432856324, 0.0318960326156814, 0.0331047661035236, 0.0343398063312275, 0.0356013143874111, 0.0368894499032755, 0.0382043710872463, 0.0395462347582974, 0.0409151963780232, 0.0423114100815264, 0.0437350287071788, 0.0451862038253117, 0.0466650857658898, 0.0481718236452158, 0.049706565391714, 0.0512694577708345, 0.0528606464091205, 0.0544802758174765, 0.0561284894136735, 0.0578054295441256, 0.0595112375049707, 0.0612460535624849, 0.0630100169728596, 0.0648032660013696, 0.0666259379409563, 0.0684781691302512, 0.070360094971063, 0.0722718499453493, 0.0742135676316953, 0.0761853807213167, 0.0781874210336082, 0.0802198195312533, 0.0822827063349132, 0.0843762107375113, 0.0865004612181274, 0.0886555854555171, 0.0908417103412699, 0.0930589619926197, 0.0953074657649191, 0.0975873462637915, 0.0998987273569704, 0.102241732185838, 0.104616483176675, 0.107023102051626, 0.109461709839399, 0.1119324268857, 0.114435372863418, 0.116970666782559, 0.119538426999953, 0.122138771228724, 0.124771816547542, 0.127437679409664, 0.130136475651761, 0.132868320502552, 0.135633328591233, 0.138431613955729, 0.141263290050755, 0.144128469755705, 0.147027265382362, 0.149959788682454, 0.152926150855031, 0.155926462553701, 0.158960833893705, 0.162029374458845, 0.16513219330827, 0.168269398983119, 0.171441099513036, 0.174647402422543, 0.17788841473729, 0.181164242990184, 0.184474993227387, 0.187820771014205, 0.191201681440861, 0.194617829128147, 0.198069318232982, 0.201556252453853, 0.205078735036156, 0.208636868777438, 0.212230756032542, 0.215860498718652, 0.219526198320249, 0.223227955893977, 0.226965872073417, 0.23074004707378, 0.23455058069651, 0.238397572333811, 0.242281120973093, 0.246201325201334, 0.250158283209375, 0.254152092796134, 0.258182851372752, 0.262250655966664, 0.266355603225604, 0.270497789421545, 0.274677310454565, 0.278894261856656, 0.283148738795466, 0.287440836077983, 0.291770648154158, 0.296138269120463, 0.300543792723403, 0.304987312362961, 0.309468921095997, 0.313988711639584, 0.3185467763743, 0.323143207347467, 0.32777809627633, 0.332451534551205, 0.337163613238559, 0.341914423084057, 0.346704054515559, 0.351532597646068, 0.356400142276637, 0.361306777899234, 0.36625259369956, 0.371237678559833, 0.376262121061519, 0.381326009488037, 0.386429431827418, 0.39157247577492, 0.396755228735618, 0.401977777826949, 0.407240209881218, 0.41254261144808, 0.417885068796976, 0.423267667919539, 0.428690494531971, 0.434153634077377, 0.439657171728079, 0.445201192387887, 0.450785780694349, 0.456411021020965, 0.462076997479369, 0.467783793921492, 0.473531493941681, 0.479320180878805, 0.485149937818323, 0.491020847594331, 0.496932992791578, 0.502886455747457, 0.50888131855397, 0.514917663059676, 0.520995570871595, 0.527115123357109, 0.533276401645826, 0.539479486631421, 0.545724458973463, 0.552011399099209, 0.558340387205378, 0.56471150325991, 0.571124827003694, 0.577580437952282, 0.584078415397575, 0.590618838409497, 0.597201785837643, 0.603827336312907, 0.610495568249093, 0.617206559844509, 0.623960389083534, 0.630757133738175, 0.637596871369601, 0.644479679329661, 0.651405634762384, 0.658374814605461, 0.665387295591707, 0.672443154250516, 0.679542466909286, 0.686685309694841, 0.693871758534824, 0.701101889159085, 0.708375777101046, 0.71569349769906, 0.723055126097739, 0.730460737249286, 0.737910405914797, 0.745404206665559, 0.752942213884326, 0.760524501766589, 0.768151144321824, 0.775822215374732, 0.783537788566466, 0.791297937355839, 0.799102735020525, 0.806952254658248, 0.81484656918795, 0.822785751350956, 0.830769873712124, 0.838799008660978, 0.846873228412837, 0.854992605009927, 0.863157210322481, 0.871367116049835, 0.879622393721502, 0.887923114698241, 0.896269350173118, 0.904661171172551, 0.913098648557343, 0.921581853023715, 0.930110855104312, 0.938685725169219, 0.947306533426946, 0.955973349925421, 0.964686244552961, 0.973445287039244, 0.982250546956257, 0.991102093719252, 1.0,
		};
		// clang-format on


	public:
		explicit TColor() = default;
		constexpr TColor(u8 r, u8 g, u8 b, u8 a = 255) requires(mode == ColorMode::RGBA)
		    : TColorData<mode>(r, g, b, a)
		{}
		constexpr TColor(float r, float g, float b, float a = 1.0f) requires(
		    mode == ColorMode::Linear || mode == ColorMode::sRGB)
		    : TColorData<mode>(r, g, b, a)
		{}
		constexpr TColor(float h, float s, float v, float a = 1.0f) requires(mode == ColorMode::HSV)
		    : TColorData<mode>(h, s, v, a)
		{}
		constexpr TColor(const v3& vector) requires(mode != ColorMode::RGBA)
		    : TColorData<mode>(vector.x, vector.y, vector.z, 1.0f)
		{}
		explicit constexpr TColor(const v4& vector) requires(mode != ColorMode::RGBA)
		    : TColorData<mode>(vector.x, vector.y, vector.z, vector.w)
		{}

		constexpr TColor(const TColor& other) requires(mode != ColorMode::HSV)
		    : TColorData<mode>(other.r, other.g, other.b, other.a)
		{}
		constexpr TColor(const TColor& other) requires(mode == ColorMode::HSV)
		    : TColorData<mode>(other.h, other.s, other.v, other.a)
		{}

		constexpr TColor& operator=(const TColor& other)
		{
			if constexpr (mode == ColorMode::HSV)
			{
				this->h = other.h;
				this->s = other.s;
				this->v = other.v;
			}
			else
			{
				this->r = other.r;
				this->g = other.g;
				this->b = other.b;
			}
			this->a = other.a;
			return *this;
		}
		// Conversion constructor
		template<ColorMode otherMode>
		constexpr TColor(const TColor<otherMode>& other) requires(mode != otherMode)
		{
			*this = other.template Convert<mode>();
		}
		template<ColorMode otherMode>
		constexpr TColor& operator=(const TColor<otherMode>& other) requires(mode != otherMode)
		{
			return other.template Convert<mode>();
		}


		///////////////////////////////////////////////////
		// Conversions

		template<ColorMode to>
		constexpr TColor<to> Convert() const
		{
			constexpr ColorMode from = mode;

			if constexpr (from == to)
			{
				return *this;
			}
			else if constexpr (to == ColorMode::Linear && from == ColorMode::RGBA)
			{
				return {float(this->r) * oneOver255, float(this->g) * oneOver255,
				    float(this->b) * oneOver255, float(this->a) * oneOver255};
			}
			else if constexpr (to == ColorMode::RGBA && from == ColorMode::Linear)
			{
				return {u8(math::Clamp<i32>(i32(this->r * 255.f), 0, 255)),
				    u8(math::Clamp<i32>(i32(this->g * 255.f), 0, 255)),
				    u8(math::Clamp<i32>(i32(this->b * 255.f), 0, 255)),
				    u8(math::Clamp<i32>(i32(this->a * 255.f), 0, 255))};
			}
			else if constexpr (to == ColorMode::sRGB && from == ColorMode::RGBA)
			{
				return {float(sRGBToLinearTable[this->r]), float(sRGBToLinearTable[this->g]),
				    float(sRGBToLinearTable[this->b]), float(this->a) * oneOver255};
			}
			else if constexpr (to == ColorMode::RGBA && from == ColorMode::sRGB)
			{
				float floatr = math::Clamp(this->r, 0.0f, 1.0f);
				float floatg = math::Clamp(this->g, 0.0f, 1.0f);
				float floatb = math::Clamp(this->b, 0.0f, 1.0f);
				float floata = math::Clamp(this->a, 0.0f, 1.0f);

				constexpr float exp = 1.0f / 2.4f;
				floatr              = floatr <= 0.0031308f ? floatr * 12.92f
				                                           : math::Pow(floatr, exp) * 1.055f - 0.055f;
				floatg              = floatg <= 0.0031308f ? floatg * 12.92f
				                                           : math::Pow(floatg, exp) * 1.055f - 0.055f;
				floatb              = floatb <= 0.0031308f ? floatb * 12.92f
				                                           : math::Pow(floatb, exp) * 1.055f - 0.055f;

				return {u8(floatr * 255.999f), u8(floatg * 255.999f), u8(floatb * 255.999f),
				    u8(floata * 255.999f)};
			}
			else if constexpr (to == ColorMode::HSV && from == ColorMode::Linear)
			{
				const float rgbMin   = math::Min(this->r, this->g, this->b);
				const float rgbMax   = math::Max(this->r, this->g, this->b);
				const float rgbRange = rgbMax - rgbMin;

				const float hue =
				    (rgbMax == rgbMin ? 0.0f
				        : rgbMax == this->r
				            ? math::Mod((((this->g - this->b) / rgbRange) * 60.0f) + 360.0f, 360.0f)
				        : rgbMax == this->g ? (((this->b - this->r) / rgbRange) * 60.0f) + 120.0f
				        : rgbMax == this->b ? (((this->r - this->g) / rgbRange) * 60.0f) + 240.0f
				                            : 0.0f);

				const float saturation = (rgbMax == 0.0f ? 0.0f : rgbRange / rgbMax);
				const float value      = rgbMax;
				return {hue, saturation, value, this->a};
			}
			else if constexpr (to == ColorMode::Linear && from == ColorMode::HSV)
			{
				const float hDiv60         = this->h / 60.0f;
				const float hDiv60Floor    = math::Floor(hDiv60);
				const float hDiv60Fraction = hDiv60 - hDiv60Floor;

				const u32 swizzleIndex = u32(hDiv60Floor) % 6;

				constexpr u32 rgbSwizzle[6][3] = {
				    {0, 3, 1},
                    {2, 0, 1},
                    {1, 0, 3},
                    {1, 2, 0},
                    {3, 1, 0},
                    {0, 1, 2}
                };
				const float rgbValues[4] = {
				    this->v,
				    this->v * (1.0f - this->h),
				    this->v * (1.0f - (hDiv60Fraction * this->h)),
				    this->v * (1.0f - ((1.0f - hDiv60Fraction) * this->h)),
				};
				return {rgbValues[rgbSwizzle[swizzleIndex][0]],
				    rgbValues[rgbSwizzle[swizzleIndex][1]], rgbValues[rgbSwizzle[swizzleIndex][2]],
				    this->a};
			}
			else
			{
				CheckMsg(false, "Not supported color conversion");
				return TColor<to>{};
			}
		}

		/*template<typename T>
		constexpr T Convert<T::mode>() const
		{
		    return Convert<T::mode>;
		}*/

		static constexpr TColor FromRGB(u8 r, u8 g, u8 b, u8 a = 255)
		{
			const TColor<ColorMode::RGBA> color{r, g, b, a};
			return TColor{color};
		}
		static constexpr TColor FromHEX(u32 value)
		{
			const TColor<ColorMode::RGBA> color{u8(value >> 16), u8(value >> 8), u8(value >> 0)};
			return TColor{color};
		}

		// Same as FromHEX() but taking into account alpha value
		static constexpr TColor HexAlpha(u32 value)
		{
			const TColor<ColorMode::RGBA> color{
			    u8(value >> 24), u8(value >> 16), u8(value >> 8), u8(value >> 0)};
			return TColor{color};
		}

		static TColor<mode> MakeFromHSV8(u8 hue, u8 saturation, u8 value)
		{
			// want a given hue value of 255 to map to just below 360 degrees
			const TColor<ColorMode::HSV> hsvColor{float(hue) * 360.0f * oneOver255,
			    float(saturation) * oneOver255, float(value) * oneOver255};
			return hsvColor.Convert<mode>();
		}

		constexpr u32& DWColor() requires(mode == ColorMode::RGBA)
		{
			return *((u32*)this);
		}
		constexpr const u32& DWColor() const requires(mode == ColorMode::RGBA)
		{
			return *((u32*)this);
		}

		// Gets the color in a packed u32 format packed in the order ARGB.
		constexpr u32 ToPackedARGB() const requires(mode == ColorMode::RGBA)
		{
			return (this->a << 24) | (this->r << 16) | (this->g << 8) | (this->b << 0);
		}

		// Gets the color in a packed u32 format packed in the order ABGR.
		constexpr u32 ToPackedABGR() const requires(mode == ColorMode::RGBA)
		{
			return (this->a << 24) | (this->b << 16) | (this->g << 8) | (this->r << 0);
		}

		// Gets the color in a packed u32 format packed in the order RGBA.
		constexpr u32 ToPackedRGBA() const requires(mode == ColorMode::RGBA)
		{
			return (this->r << 24) | (this->g << 16) | (this->g << 8) | (this->a << 0);
		}

		// Gets the color in a packed u32 format packed in the order BGRA.
		constexpr u32 ToPackedBGRA() const requires(mode == ColorMode::RGBA)
		{
			return (this->b << 24) | (this->g << 16) | (this->r << 8) | (this->a << 0);
		}


		///////////////////////////////////////////////////
		// Operators

		constexpr TColor operator+(const TColor& other) const
		{
			if constexpr (mode == ColorMode::RGBA)
			{
				return {u8(math::Clamp(i32(this->r) + other.r, 0, 255)),
				    u8(math::Clamp(i32(this->g) + other.g, 0, 255)),
				    u8(math::Clamp(i32(this->b) + other.b, 0, 255)),
				    u8(math::Clamp(i32(this->a) + other.a, 0, 255))};
			}
			else if constexpr (mode == ColorMode::HSV)
			{
				CheckMsg(false, "operator+(color) is not allowed on HSV");
				return {};
			}
			return {this->r + other.r, this->g + other.g, this->b + other.b, this->a + other.a};
		}
		constexpr TColor& operator+=(const TColor& other)
		{
			*this = *this + other;
			return *this;
		}
		constexpr TColor operator-(const TColor& other) const
		{
			if constexpr (mode == ColorMode::RGBA)
			{
				return {u8(math::Clamp(i32(this->r) - other.r, 0, 255)),
				    u8(math::Clamp(i32(this->g) - other.g, 0, 255)),
				    u8(math::Clamp(i32(this->b) - other.b, 0, 255)),
				    u8(math::Clamp(i32(this->a) - other.a, 0, 255))};
			}
			else if constexpr (mode == ColorMode::HSV)
			{
				CheckMsg(false, "operator-(color) is not allowed on HSV");
				return {};
			}
			return {this->r - other.r, this->g - other.g, this->b - other.b, this->a - other.a};
		}
		constexpr TColor& operator-=(const TColor& other)
		{
			*this = *this - other;
			return *this;
		}
		constexpr TColor operator*(const TColor& other) const
		{
			if constexpr (mode == ColorMode::RGBA)
			{
				return {u8(math::Clamp(i32(this->r) * other.r, 0, 255)),
				    u8(math::Clamp(i32(this->g) * other.g, 0, 255)),
				    u8(math::Clamp(i32(this->b) * other.b, 0, 255)),
				    u8(math::Clamp(i32(this->a) * other.a, 0, 255))};
			}
			else if constexpr (mode == ColorMode::HSV)
			{
				CheckMsg(false, "operator*(color) is not allowed on HSV");
				return {};
			}
			return {this->r * other.r, this->g * other.g, this->b * other.b, this->a * other.a};
		}
		constexpr TColor& operator*=(const TColor& other)
		{
			*this = *this * other;
			return *this;
		}
		constexpr TColor operator/(const TColor& other) const
		{
			if constexpr (mode == ColorMode::RGBA)
			{
				return {u8(math::Clamp(i32(this->r) / other.r, 0, 255)),
				    u8(math::Clamp(i32(this->g) / other.g, 0, 255)),
				    u8(math::Clamp(i32(this->b) / other.b, 0, 255)),
				    u8(math::Clamp(i32(this->a) / other.a, 0, 255))};
			}
			else if constexpr (mode == ColorMode::HSV)
			{
				CheckMsg(false, "operator/(color) is not allowed on HSV");
				return {};
			}
			return {this->r / other.r, this->g / other.g, this->b / other.b, this->a / other.a};
		}
		constexpr TColor& operator/=(const TColor& other)
		{
			*this = *this / other;
			return *this;
		}
		constexpr TColor operator*(float scalar) const
		{
			if constexpr (mode == ColorMode::RGBA)
			{
				return {u8(math::Clamp(i32(scalar * this->r), 0, 255)),
				    u8(math::Clamp(i32(scalar * this->g), 0, 255)),
				    u8(math::Clamp(i32(scalar * this->b), 0, 255)),
				    u8(math::Clamp(i32(scalar * this->a), 0, 255))};
			}
			else if constexpr (mode == ColorMode::HSV)
			{
				CheckMsg(false, "operator*(scalar) is not allowed on HSV");
				return {};
			}
			return {this->r * scalar, this->g * scalar, this->b * scalar, this->a * scalar};
		}
		constexpr TColor& operator*=(float scalar)
		{
			*this = *this * scalar;
			return *this;
		}
		constexpr TColor operator/(float scalar) const
		{
			const float invScalar = 1.0f / scalar;
			if constexpr (mode == ColorMode::RGBA)
			{
				return {u8(math::Clamp(i32(invScalar * this->r), 0, 255)),
				    u8(math::Clamp(i32(invScalar * this->g), 0, 255)),
				    u8(math::Clamp(i32(invScalar * this->b), 0, 255)),
				    u8(math::Clamp(i32(invScalar * this->a), 0, 255))};
			}
			else if constexpr (mode == ColorMode::HSV)
			{
				CheckMsg(false, "operator/(scalar) is not allowed on HSV");
				return {};
			}
			return {
			    this->r * invScalar, this->g * invScalar, this->b * invScalar, this->a * invScalar};
		}
		constexpr TColor& operator/=(float scalar)
		{
			*this = *this / scalar;
			return *this;
		}

		constexpr bool operator==(const TColor& other) const
		{
			if constexpr (mode == ColorMode::RGBA)
			{
				return DWColor() == other.DWColor();
			}
			return this->r == other.r && this->g == other.g && this->b == other.b
			    && this->a == other.a;
		}
		constexpr bool operator!=(const TColor& other) const
		{
			if constexpr (mode != ColorMode::RGBA)
			{
				return DWColor() == other.DWColor();
			}
			return this->r != other.r || this->g != other.g || this->b != other.b
			    || this->a != other.a;
		}

		constexpr auto* Data()
		{
			if constexpr (mode == ColorMode::HSV)
				return &this->h;
			else
				return &this->r;
		}
		constexpr const auto* Data() const
		{
			if constexpr (mode == ColorMode::HSV)
				return &this->h;
			else
				return &this->r;
		}
		constexpr auto& operator[](u32 i)
		{
			return Data()[i];
		}
		constexpr const auto& operator[](u32 i) const
		{
			return Data()[i];
		}

		TColor Clamp(float min = 0.f, float max = 1.f) const requires(mode != ColorMode::RGBA)
		{
			return {math::Clamp(this->r, min, max), math::Clamp(this->g, min, max),
			    math::Clamp(this->b, min, max), math::Clamp(this->a, min, max)};
		}

		// Error-tolerant comparison.
		bool Equals(const TColor& other, float Tolerance = math::SMALL_NUMBER) const
		    requires(mode != ColorMode::RGBA)
		{
			return math::Abs(this->r - other.r) < Tolerance
			    && math::Abs(this->g - other.g) < Tolerance
			    && math::Abs(this->b - other.b) < Tolerance
			    && math::Abs(this->a - other.a) < Tolerance;
		}

		constexpr v3_u8 ToV3() const requires(mode == ColorMode::RGBA)
		{
			return {this->r, this->g, this->b};
		}
		constexpr v4_u8 ToV4() const requires(mode == ColorMode::RGBA)
		{
			return {this->r, this->g, this->b, this->a};
		}
		constexpr v3 ToV3() const requires(mode != ColorMode::RGBA)
		{
			return {this->r, this->g, this->b};
		}
		constexpr v4 ToV4() const requires(mode != ColorMode::RGBA)
		{
			return {this->r, this->g, this->b, this->a};
		}


		/**
		 * Makes a random but quite nice color
		 */
		static TColor MakeRandomColor();

		/**
		 * Converts temperature in Kelvins of a black body radiator to RGB chromaticism.
		 */
		static TColor MakeFromColorTemperature(float temp);

		/**
		 * Euclidean distance between two points.
		 */
		static inline float Dist(const TColor& one, const TColor& other)
		{
			return math::Sqrt(math::Square(other.r - one.r) + math::Square(other.g - one.g)
			                  + math::Square(other.b - one.b) + math::Square(other.a - one.a));
		}

		/**
		 * Linearly interpolates between two colors by the specified progress amount.  The
		 * interpolation is performed in HSV color space taking the shortest path to the new color's
		 * hue.  This can give better results than math::Lerp(), but is much more expensive. The
		 * incoming colors are in RGB space, and the output color will be RGB.  The alpha value will
		 * also be interpolated.
		 *
		 * @param	from	The color and alpha to interpolate from as linear RGBA
		 * @param	to		The color and alpha to interpolate to as linear RGBA
		 * @param	delta	scalar interpolation amount (usually between 0.0 and 1.0 inclusive)
		 * @return	The interpolated color in linear RGB space along with the interpolated alpha
		 * value
		 */
		static TColor<mode> LerpUsingHSV(
		    const TColor<mode>& from, const TColor<mode>& to, float delta)
		{
			if constexpr (mode != ColorMode::HSV)
			{
				return TColor<ColorMode::HSV>::LerpUsingHSV(from.template Convert<ColorMode::HSV>(),
				    to.template Convert<ColorMode::HSV>(), delta)
				    .template Convert<mode>();
			}
			else
			{
				float fromHue = from.h;
				float toHue   = to.h;

				// Take the shortest path to the new hue
				if (math::Abs(fromHue - toHue) > 180.0f)
				{
					if (toHue > fromHue)
					{
						fromHue += 360.0f;
					}
					else
					{
						toHue += 360.0f;
					}
				}

				float newHue = math::Lerp(fromHue, toHue, delta);
				newHue       = math::Mod(newHue, 360.0f);
				if (newHue < 0.0f)
				{
					newHue += 360.0f;
				}

				return {newHue, math::Lerp(from.s, to.s, delta), math::Lerp(from.v, to.v, delta),
				    math::Lerp(from.a, to.a, delta)};
			}
		}

		/**
		 * Returns a desaturated color, with 0 meaning no desaturation and 1 == full desaturation
		 * @param	Desaturation	Desaturation factor in range [0..1]
		 * @return	Desaturated color
		 */
		constexpr TColor Desaturate(float desaturation) const
		    requires(mode == ColorMode::Linear || mode == ColorMode::sRGB)
		{
			float lum = ComputeLuminance();
			return math::Lerp(*this, TColor<mode>{lum, lum, lum, this->a}, desaturation);
		}
		constexpr TColor Shade(float delta) const;
		constexpr TColor Tint(float delta) const;
		constexpr TColor Translucency(u8 alpha) const requires(mode == ColorMode::RGBA)
		{
			return {this->r, this->g, this->b, alpha};
		}
		constexpr TColor Translucency(float alpha) const requires(mode != ColorMode::RGBA)
		{
			return {this->r, this->g, this->b, math::Clamp(alpha, 0.f, 1.f)};
		}

		/** Computes the perceptually weighted luminance value of a color. */
		constexpr float ComputeLuminance() const
		{
			return this->r * 0.3f + this->g * 0.59f + this->b * 0.11f;
		}

		/**
		 * Returns the maximum value in this color structure
		 * @return The maximum color channel value
		 */
		float GetMax() const
		{
			return math::Max(math::Max(math::Max(this->r, this->g), this->b), this->a);
		}

		/** useful to detect if a light contribution needs to be rendered */
		bool IsAlmostBlack() const
		{
			return math::Square(this->r) < math::SMALLER_NUMBER
			    && math::Square(this->g) < math::SMALLER_NUMBER
			    && math::Square(this->b) < math::SMALLER_NUMBER;
		}

		/**
		 * Returns the minimum value in this color structure
		 * @return The minimum color channel value
		 */
		float GetMin() const
		{
			return math::Min(math::Min(math::Min(this->r, this->g), this->b), this->a);
		}

		float GetLuminance() const
		{
			return this->r * 0.3f + this->g * 0.59f + this->b * 0.11f;
		}

		/**
		 * Converts a color into string
		 * @return resulting string
		 * @see ToHex
		 */
		String ToString() const
		{
			if constexpr (mode == ColorMode::HSV)
			{
				return Strings::Format(
				    TX("(h={},s={},v={},a={})"), this->h, this->s, this->v, this->a);
			}
			return Strings::Format(TX("(r={},g={},b={},a={})"), this->r, this->g, this->b, this->a);
		}

		/**
		 * Converts this color value to a hexadecimal string.
		 * The format of the string is RRGGBBAA.
		 * @return Hexadecimal string.
		 * @see FromHex, ToString
		 */
		String ToHex(bool includeAlpha = true) const requires(mode == ColorMode::RGBA)
		{
			if (includeAlpha)
			{
				return Strings::Format(
				    TX("{:02X}{:02X}{:02X}{:02X}"), this->r, this->g, this->b, this->a);
			}
			return Strings::Format(TX("{:02X}{:02X}{:02X}"), this->r, this->g, this->b);
		}

		// Common colors
		static constexpr TColor White()
		{
			return FromRGB(255, 255, 255);
		};
		static constexpr TColor Gray()
		{
			return FromRGB(127, 127, 127);
		};
		static constexpr TColor Black()
		{
			return FromRGB(0, 0, 0);
		};
		static constexpr TColor Transparent()
		{
			return FromRGB(0, 0, 0, 0);
		};
		static constexpr TColor Red()
		{
			return FromRGB(255, 0, 0);
		};
		static constexpr TColor Green()
		{
			return FromRGB(0, 255, 0);
		};
		static constexpr TColor Blue()
		{
			return FromRGB(0, 0, 255);
		};
		static constexpr TColor Yellow()
		{
			return FromRGB(255, 255, 0);
		};
		static constexpr TColor Cyan()
		{
			return FromRGB(0, 255, 255);
		};
		static constexpr TColor Magenta()
		{
			return FromRGB(255, 0, 255);
		};
		static constexpr TColor Orange()
		{
			return FromRGB(243, 156, 18);
		};
		static constexpr TColor Purple()
		{
			return FromRGB(169, 7, 228);
		};
		static constexpr TColor Turquoise()
		{
			return FromRGB(26, 188, 156);
		};
		static constexpr TColor Silver()
		{
			return FromRGB(189, 195, 199);
		};
		static constexpr TColor Emerald()
		{
			return FromRGB(46, 204, 113);
		};
	};

	template<ColorMode mode>
	constexpr TColor<mode> operator*(float scalar, const TColor<mode>& color)
	{
		return color.operator*(scalar);
	}


	using LinearColor = TColor<ColorMode::Linear>;
	using sRGBColor   = TColor<ColorMode::sRGB>;
	using HSVColor    = TColor<ColorMode::HSV>;
	using Color       = TColor<ColorMode::RGBA>;


	/** Computes a brightness and a fixed point color from a floating point color. */
	extern constexpr void ComputeAndFixedColorAndIntensity(
	    LinearColor& linearColor, Color& outColor, float& outIntensity)
	{
		const float maxComponent =
		    math::Max(math::SMALLER_NUMBER, linearColor.r, linearColor.g, linearColor.b);
		outColor     = (linearColor / maxComponent).Convert<ColorMode::RGBA>();
		outIntensity = maxComponent;
	}


	////////////////////////////////////////////////////////
	// TColor inline functions

	template<ColorMode mode>
	TColor<mode> TColor<mode>::MakeRandomColor()
	{
		const u8 hue = u8(math::Rand01() * 255.f);
		return MakeFromHSV8(hue, 0, 255);
	}

	template<ColorMode mode>
	TColor<mode> TColor<mode>::MakeFromColorTemperature(float temp)
	{
		temp = math::Clamp(temp, 1000.0f, 15000.0f);

		// Approximate Planckian locus in CIE 1960 UCS
		const float u = (0.860117757f + 1.54118254e-4f * temp + 1.28641212e-7f * temp * temp)
		              / (1.0f + 8.42420235e-4f * temp + 7.08145163e-7f * temp * temp);
		const float v = (0.317398726f + 4.22806245e-5f * temp + 4.20481691e-8f * temp * temp)
		              / (1.0f - 2.89741816e-5f * temp + 1.61456053e-7f * temp * temp);

		const float invDiv = 1.f / (2.f * u - 8.f * v + 4.f);
		const float x      = 3.0f * u * invDiv;
		const float y      = 2.0f * v * invDiv;
		const float z      = 1.0f - x - y;

		const float Y = 1.0f;
		const float X = Y / y * x;
		const float Z = Y / y * z;

		// XYZ to RGB with BT.709 primaries
		const float r = 3.2404542f * X + -1.5371385f * Y + -0.4985314f * Z;
		const float g = -0.9692660f * X + 1.8760108f * Y + 0.0415560f * Z;
		const float b = 0.0556434f * X + -0.2040259f * Y + 1.0572252f * Z;
		return TColor<ColorMode::sRGB>{r, g, b}.Convert<mode>();
	}

	template<ColorMode mode>
	constexpr TColor<mode> TColor<mode>::Shade(float delta) const
	{
		Color tmp{*this};
		tmp.r = u8(math::Lerp<i32>(tmp.r, Color::Black().r, delta));
		tmp.g = u8(math::Lerp<i32>(tmp.g, Color::Black().g, delta));
		tmp.b = u8(math::Lerp<i32>(tmp.b, Color::Black().b, delta));
		tmp.a = tmp.a;
		return tmp.Convert<mode>();
	}

	template<ColorMode mode>
	constexpr TColor<mode> TColor<mode>::Tint(float delta) const
	{
		Color tmp{*this};
		tmp.r = u8(math::Lerp<i32>(tmp.r, Color::White().r, delta));
		tmp.g = u8(math::Lerp<i32>(tmp.g, Color::White().g, delta));
		tmp.b = u8(math::Lerp<i32>(tmp.b, Color::White().b, delta));
		tmp.a = tmp.a;
		return tmp.Convert<mode>();
	}


	template<ColorMode mode>
	inline void Read(Reader& ct, TColor<mode>& color)
	{
		ct.BeginObject();
		ct.Next("r", color.r);
		ct.Next("g", color.g);
		ct.Next("b", color.b);
		ct.Next("a", color.a);
	}

	template<ColorMode mode>
	inline void Write(Writer& ct, const TColor<mode>& color)
	{
		ct.BeginObject();
		ct.Next("r", color.r);
		ct.Next("g", color.g);
		ct.Next("b", color.b);
		ct.Next("a", color.a);
	}

	template<>
	inline void Read(Reader& ct, TColor<ColorMode::HSV>& color)
	{
		ct.BeginObject();
		ct.Next("h", color.h);
		ct.Next("s", color.s);
		ct.Next("v", color.v);
		ct.Next("a", color.a);
	}

	template<>
	inline void Write(Writer& ct, const TColor<ColorMode::HSV>& color)
	{
		ct.BeginObject();
		ct.Next("h", color.h);
		ct.Next("s", color.s);
		ct.Next("v", color.v);
		ct.Next("a", color.a);
	}


	template<ColorMode mode>
	struct Hash<TColor<mode>>
	{
		sizet operator()(const TColor<mode>& color) const
		{
			return HashBytes(&color, sizeof(TColor<mode>));
		}
	};

	template<>
	struct Hash<TColor<ColorMode::RGBA>>
	{
		sizet operator()(const TColor<ColorMode::RGBA>& color) const
		{
			return color.DWColor();
		}
	};
}    // namespace p


REFLECT_NATIVE_TYPE(p::LinearColor)
REFLECT_NATIVE_TYPE(p::sRGBColor)
REFLECT_NATIVE_TYPE(p::HSVColor)
REFLECT_NATIVE_TYPE(p::Color)
