// Copyright 1998-2018 Epic Games, Inother. All Rights Reserved.

#pragma once

#include "PCH.h"

#include "Containers/Array.h"
#include "Math.h"
#include "Reflection/Static/NativeType.h"
#include "Reflection/TypeFlags.h"
#include "Serialization/ContextsFwd.h"
#include "Strings/String.h"
#include "Vector.h"


namespace Rift
{
	/**
	 * Enum for the different kinds of gamma spaces we expect to need to convert from/to.
	 */
	enum class EGammaSpace
	{
		/** No gamma correction is applied to this space, the incoming colors are assumed to already
		   be in linear space. */
		Linear,
		/** A simplified sRGB gamma correction is applied, pow(1/2.2). */
		Pow22,
		/** Use the standard sRGB conversion. */
		sRGB,
	};

	struct Color;


	/**
	 * A linear, 32-bit/component floating point RGBA color.
	 */
	struct CORE_API LinearColor
	{
		float r = 0.f, g = 0.f, b = 0.f, a = 0.f;

		// clang-format off
		/**
		 * Pow table for fast Color -> FLinearColor conversion.
		 *
		 * Math::Pow( i / 255.f, 2.2f )
		 */
		static constexpr double Pow22OneOver255Table[256] {
			0, 5.07705190066176E-06, 2.33280046660989E-05, 5.69217657121931E-05, 0.000107187362341244, 0.000175123977503027, 0.000261543754548491, 0.000367136269815943, 0.000492503787191433, 0.000638182842167022, 0.000804658499513058, 0.000992374304074325, 0.0012017395224384, 0.00143313458967186, 0.00168691531678928, 0.00196341621339647, 0.00226295316070643, 0.00258582559623417, 0.00293231832393836, 0.00330270303200364, 0.00369723957890013, 0.00411617709328275, 0.00455975492252602, 0.00502820345685554, 0.00552174485023966, 0.00604059365484981, 0.00658495738258168, 0.00715503700457303, 0.00775102739766061, 0.00837311774514858, 0.00902149189801213, 0.00969632870165823, 0.0103978022925553, 0.0111260823683832, 0.0118813344348137, 0.0126637200315821, 0.0134733969401426, 0.0143105193748841, 0.0151752381596252, 0.0160677008908869, 0.01698805208925, 0.0179364333399502, 0.0189129834237215, 0.0199178384387857, 0.0209511319147811, 0.0220129949193365, 0.0231035561579214, 0.0242229420675342, 0.0253712769047346, 0.0265486828284729, 0.027755279978126, 0.0289911865471078, 0.0302565188523887, 0.0315513914002264, 0.0328759169483838, 0.034230206565082, 0.0356143696849188, 0.0370285141619602, 0.0384727463201946, 0.0399471710015256, 0.0414518916114625, 0.0429870101626571, 0.0445526273164214, 0.0461488424223509, 0.0477757535561706, 0.049433457555908, 0.0511220500564934, 0.052841625522879, 0.0545922772817603, 0.0563740975519798, 0.0581871774736854, 0.0600316071363132, 0.0619074756054558, 0.0638148709486772, 0.0657538802603301, 0.0677245896854243, 0.0697270844425988, 0.0717614488462391, 0.0738277663277846, 0.0759261194562648, 0.0780565899581019, 0.080219258736215, 0.0824142058884592, 0.0846415107254295, 0.0869012517876603, 0.0891935068622478, 0.0915183529989195, 0.0938758665255778, 0.0962661230633397, 0.0986891975410945, 0.1011451642096, 0.103634096655137, 0.106156067812744, 0.108711149979039, 0.11129941482466, 0.113920933406333, 0.116575776178572, 0.119264013005047, 0.121985713169619, 0.124740945387051, 0.127529777813422, 0.130352278056244, 0.1332085131843, 0.136098549737202, 0.139022453734703, 0.141980290685736, 0.144972125597231, 0.147998022982685, 0.151058046870511, 0.154152260812165, 0.157280727890073, 0.160443510725344, 0.16364067148529, 0.166872271890766, 0.170138373223312, 0.173439036332135, 0.176774321640903, 0.18014428915439, 0.183548998464951, 0.186988508758844, 0.190462878822409, 0.193972167048093, 0.19751643144034, 0.201095729621346, 0.204710118836677, 0.208359655960767, 0.212044397502288, 0.215764399609395, 0.219519718074868, 0.223310408341127, 0.227136525505149, 0.230998124323267, 0.23489525921588, 0.238827984272048, 0.242796353254002, 0.24680041960155, 0.2508402364364, 0.254915856566385, 0.259027332489606, 0.263174716398492, 0.267358060183772, 0.271577415438375, 0.275832833461245, 0.280124365261085, 0.284452061560024, 0.288815972797219, 0.293216149132375, 0.297652640449211, 0.302125496358853, 0.306634766203158, 0.311180499057984, 0.315762743736397, 0.32038154879181, 0.325036962521076, 0.329729032967515, 0.334457807923889, 0.339223334935327, 0.344025661302187, 0.348864834082879, 0.353740900096629, 0.358653905926199, 0.363603897920553, 0.368590922197487, 0.373615024646202, 0.37867625092984, 0.383774646487975, 0.388910256539059, 0.394083126082829, 0.399293299902674, 0.404540822567962, 0.409825738436323, 0.415148091655907, 0.420507926167587, 0.425905285707146, 0.43134021380741, 0.436812753800359, 0.442322948819202, 0.44787084180041, 0.453456475485731, 0.45907989242416, 0.46474113497389, 0.470440245304218, 0.47617726539744, 0.481952237050698, 0.487765201877811, 0.493616201311074, 0.49950527660303, 0.505432468828216, 0.511397818884879, 0.517401367496673, 0.523443155214325, 0.529523222417277, 0.535641609315311, 0.541798355950137, 0.547993502196972, 0.554227087766085, 0.560499152204328, 0.566809734896638, 0.573158875067523, 0.579546611782525, 0.585972983949661, 0.592438030320847, 0.598941789493296, 0.605484299910907, 0.612065599865624, 0.61868572749878, 0.625344720802427, 0.632042617620641, 0.638779455650817, 0.645555272444934, 0.652370105410821, 0.659223991813387, 0.666116968775851, 0.673049073280942, 0.680020342172095, 0.687030812154625, 0.694080519796882, 0.701169501531402, 0.708297793656032, 0.715465432335048, 0.722672453600255, 0.729918893352071, 0.737204787360605, 0.744530171266715, 0.751895080583051, 0.759299550695091, 0.766743616862161, 0.774227314218442, 0.781750677773962, 0.789313742415586, 0.796916542907978, 0.804559113894567, 0.81224148989849, 0.819963705323528, 0.827725794455034, 0.835527791460841, 0.843369730392169, 0.851251645184515, 0.859173569658532, 0.867135537520905, 0.875137582365205, 0.883179737672745, 0.891262036813419, 0.899384513046529, 0.907547199521614, 0.915750129279253, 0.923993335251873, 0.932276850264543, 0.940600707035753, 0.948964938178195, 0.957369576199527, 0.96581465350313, 0.974300202388861, 0.982826255053791, 0.99139284359294, 1
		};

		/**
		 * Table for fast Color -> FLinearColor conversion.
		 *
		 * Color > 0.04045 ? pow( Color * (1.0 / 1.055) + 0.0521327, 2.4 ) : Color * (1.0 / 12.92);
		 */
		static constexpr double sRGBToLinearTable[256] {
		    0, 0.000303526983548838, 0.000607053967097675, 0.000910580950646512, 0.00121410793419535, 0.00151763491774419, 0.00182116190129302, 0.00212468888484186, 0.0024282158683907, 0.00273174285193954, 0.00303526983548838, 0.00334653564113713, 0.00367650719436314, 0.00402471688178252, 0.00439144189356217, 0.00477695332960869, 0.005181516543916, 0.00560539145834456, 0.00604883284946662, 0.00651209061157708, 0.00699540999852809, 0.00749903184667767, 0.00802319278093555, 0.0085681254056307, 0.00913405848170623, 0.00972121709156193, 0.0103298227927056, 0.0109600937612386, 0.0116122449260844, 0.012286488094766, 0.0129830320714536, 0.0137020827679224, 0.0144438433080002, 0.0152085141260192, 0.0159962930597398, 0.0168073754381669, 0.0176419541646397, 0.0185002197955389, 0.0193823606149269, 0.0202885627054049, 0.0212190100154473, 0.0221738844234532, 0.02315336579873, 0.0241576320596103, 0.0251868592288862, 0.0262412214867272, 0.0273208912212394, 0.0284260390768075, 0.0295568340003534, 0.0307134432856324, 0.0318960326156814, 0.0331047661035236, 0.0343398063312275, 0.0356013143874111, 0.0368894499032755, 0.0382043710872463, 0.0395462347582974, 0.0409151963780232, 0.0423114100815264, 0.0437350287071788, 0.0451862038253117, 0.0466650857658898, 0.0481718236452158, 0.049706565391714, 0.0512694577708345, 0.0528606464091205, 0.0544802758174765, 0.0561284894136735, 0.0578054295441256, 0.0595112375049707, 0.0612460535624849, 0.0630100169728596, 0.0648032660013696, 0.0666259379409563, 0.0684781691302512, 0.070360094971063, 0.0722718499453493, 0.0742135676316953, 0.0761853807213167, 0.0781874210336082, 0.0802198195312533, 0.0822827063349132, 0.0843762107375113, 0.0865004612181274, 0.0886555854555171, 0.0908417103412699, 0.0930589619926197, 0.0953074657649191, 0.0975873462637915, 0.0998987273569704, 0.102241732185838, 0.104616483176675, 0.107023102051626, 0.109461709839399, 0.1119324268857, 0.114435372863418, 0.116970666782559, 0.119538426999953, 0.122138771228724, 0.124771816547542, 0.127437679409664, 0.130136475651761, 0.132868320502552, 0.135633328591233, 0.138431613955729, 0.141263290050755, 0.144128469755705, 0.147027265382362, 0.149959788682454, 0.152926150855031, 0.155926462553701, 0.158960833893705, 0.162029374458845, 0.16513219330827, 0.168269398983119, 0.171441099513036, 0.174647402422543, 0.17788841473729, 0.181164242990184, 0.184474993227387, 0.187820771014205, 0.191201681440861, 0.194617829128147, 0.198069318232982, 0.201556252453853, 0.205078735036156, 0.208636868777438, 0.212230756032542, 0.215860498718652, 0.219526198320249, 0.223227955893977, 0.226965872073417, 0.23074004707378, 0.23455058069651, 0.238397572333811, 0.242281120973093, 0.246201325201334, 0.250158283209375, 0.254152092796134, 0.258182851372752, 0.262250655966664, 0.266355603225604, 0.270497789421545, 0.274677310454565, 0.278894261856656, 0.283148738795466, 0.287440836077983, 0.291770648154158, 0.296138269120463, 0.300543792723403, 0.304987312362961, 0.309468921095997, 0.313988711639584, 0.3185467763743, 0.323143207347467, 0.32777809627633, 0.332451534551205, 0.337163613238559, 0.341914423084057, 0.346704054515559, 0.351532597646068, 0.356400142276637, 0.361306777899234, 0.36625259369956, 0.371237678559833, 0.376262121061519, 0.381326009488037, 0.386429431827418, 0.39157247577492, 0.396755228735618, 0.401977777826949, 0.407240209881218, 0.41254261144808, 0.417885068796976, 0.423267667919539, 0.428690494531971, 0.434153634077377, 0.439657171728079, 0.445201192387887, 0.450785780694349, 0.456411021020965, 0.462076997479369, 0.467783793921492, 0.473531493941681, 0.479320180878805, 0.485149937818323, 0.491020847594331, 0.496932992791578, 0.502886455747457, 0.50888131855397, 0.514917663059676, 0.520995570871595, 0.527115123357109, 0.533276401645826, 0.539479486631421, 0.545724458973463, 0.552011399099209, 0.558340387205378, 0.56471150325991, 0.571124827003694, 0.577580437952282, 0.584078415397575, 0.590618838409497, 0.597201785837643, 0.603827336312907, 0.610495568249093, 0.617206559844509, 0.623960389083534, 0.630757133738175, 0.637596871369601, 0.644479679329661, 0.651405634762384, 0.658374814605461, 0.665387295591707, 0.672443154250516, 0.679542466909286, 0.686685309694841, 0.693871758534824, 0.701101889159085, 0.708375777101046, 0.71569349769906, 0.723055126097739, 0.730460737249286, 0.737910405914797, 0.745404206665559, 0.752942213884326, 0.760524501766589, 0.768151144321824, 0.775822215374732, 0.783537788566466, 0.791297937355839, 0.799102735020525, 0.806952254658248, 0.81484656918795, 0.822785751350956, 0.830769873712124, 0.838799008660978, 0.846873228412837, 0.854992605009927, 0.863157210322481, 0.871367116049835, 0.879622393721502, 0.887923114698241, 0.896269350173118, 0.904661171172551, 0.913098648557343, 0.921581853023715, 0.930110855104312, 0.938685725169219, 0.947306533426946, 0.955973349925421, 0.964686244552961, 0.973445287039244, 0.982250546956257, 0.991102093719252, 1.0,
		};
		// clang-format on

		explicit LinearColor() = default;
		constexpr LinearColor(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
		/**
		 * Converts an Color which is assumed to be in sRGB space, into linear color space.
		 * @param Color The sRGB color that needs to be converted into linear space.
		 */
		constexpr LinearColor(const Color& Color);
		constexpr LinearColor(const v3& vector) : r(vector.x), g(vector.y), b(vector.z), a(1.0f) {}
		explicit constexpr LinearColor(const v4& vector)
		    : r(vector.x)
		    , g(vector.y)
		    , b(vector.z)
		    , a(vector.w)
		{}

		// Conversions.
		Color ToRGBE() const;

		/**
		 * Converts an Color coming from an observed sRGB output, into a linear color.
		 * @param Color The sRGB color that needs to be converted into linear space.
		 */
		static LinearColor FromSRGBColor(const Color& Color);

		/**
		 * Converts an Color coming from an observed Pow(1/2.2) output, into a linear color.
		 * @param Color The Pow(1/2.2) color that needs to be converted into linear space.
		 */
		static LinearColor FromPow22Color(const Color& Color);

		// Operators.

		float& Component(i32 index)
		{
			return (&r)[index];
		}
		const float& Component(i32 index) const
		{
			return (&r)[index];
		}

		LinearColor operator+(const LinearColor& other) const
		{
			return LinearColor(r + other.r, g + other.g, b + other.b, a + other.a);
		}
		LinearColor& operator+=(const LinearColor& other)
		{
			r += other.r;
			g += other.g;
			b += other.b;
			a += other.a;
			return *this;
		}

		LinearColor operator-(const LinearColor& other) const
		{
			return LinearColor(r - other.r, g - other.g, b - other.b, a - other.a);
		}
		LinearColor& operator-=(const LinearColor& other)
		{
			r -= other.r;
			g -= other.g;
			b -= other.b;
			a -= other.a;
			return *this;
		}

		LinearColor operator*(const LinearColor& other) const
		{
			return LinearColor(r * other.r, g * other.g, b * other.b, a * other.a);
		}
		LinearColor& operator*=(const LinearColor& other)
		{
			r *= other.r;
			g *= other.g;
			b *= other.b;
			a *= other.a;
			return *this;
		}

		LinearColor operator*(float scalar) const
		{
			return LinearColor(r * scalar, g * scalar, b * scalar, a * scalar);
		}

		LinearColor& operator*=(float scalar)
		{
			r *= scalar;
			g *= scalar;
			b *= scalar;
			a *= scalar;
			return *this;
		}

		LinearColor operator/(const LinearColor& other) const
		{
			return LinearColor(r / other.r, g / other.g, b / other.b, a / other.a);
		}
		LinearColor& operator/=(const LinearColor& other)
		{
			r /= other.r;
			g /= other.g;
			b /= other.b;
			a /= other.a;
			return *this;
		}

		LinearColor operator/(float scalar) const
		{
			const float invScalar = 1.0f / scalar;
			return LinearColor(r * invScalar, g * invScalar, b * invScalar, a * invScalar);
		}
		LinearColor& operator/=(float scalar)
		{
			const float invScalar = 1.0f / scalar;
			r *= invScalar;
			g *= invScalar;
			b *= invScalar;
			a *= invScalar;
			return *this;
		}

		// clamped in 0..1 range
		LinearColor GetClamped(float min = 0.0f, float max = 1.0f) const
		{
			LinearColor ret;
			ret.r = Math::Clamp(r, min, max);
			ret.g = Math::Clamp(g, min, max);
			ret.b = Math::Clamp(b, min, max);
			ret.a = Math::Clamp(a, min, max);
			return ret;
		}

		/** Comparison operators */
		bool operator==(const LinearColor& other) const
		{
			return r == other.r && g == other.g && b == other.b && a == other.a;
		}
		bool operator!=(const LinearColor& Other) const
		{
			return r != Other.r || g != Other.g || b != Other.b || a != Other.a;
		}

		// Error-tolerant comparison.
		bool Equals(const LinearColor& other, float Tolerance = Math::SMALL_NUMBER) const
		{
			return Math::Abs(r - other.r) < Tolerance && Math::Abs(g - other.g) < Tolerance &&
			       Math::Abs(b - other.b) < Tolerance && Math::Abs(a - other.a) < Tolerance;
		}

		LinearColor CopyWithNewOpacity(float NewOpacity) const
		{
			LinearColor NewCopy = *this;
			NewCopy.a           = NewOpacity;
			return NewCopy;
		}

		v3 ToV3() const
		{
			return {r, g, b};
		}
		v4 ToV4() const
		{
			return {r, g, b, a};
		}

		/**
		 * Converts byte hue-saturation-brightness to floating point red-green-blue.
		 */
		static LinearColor GetHSV(u8 H, u8 S, u8 V);

		/**
		 * Makes a random but quite nice color.
		 */
		static LinearColor MakeRandomColor();

		/**
		 * Converts temperature in Kelvins of a black body radiator to RGB chromaticism.
		 */
		static LinearColor MakeFromColorTemperature(float temp);

		/**
		 * Euclidean distance between two points.
		 */
		static inline float Dist(const LinearColor& one, const LinearColor& other)
		{
			return Math::Sqrt(Math::Square(other.r - one.r) + Math::Square(other.g - one.g) +
			                  Math::Square(other.b - one.b) + Math::Square(other.a - one.a));
		}

		/**
		 * Generates a list of sample points on a Bezier curve defined by 2 points.
		 *
		 * @param	ControlPoints	Array of 4 Linear Colors (vert1, controlpoint1, controlpoint2,
		 * vert2).
		 * @param	NumPoints		Number of samples.
		 * @param	OutPoints		Receives the output samples.
		 * @return					Path length.
		 */
		static float EvaluateBezier(
		    const LinearColor* ControlPoints, i32 NumPoints, TArray<LinearColor>& OutPoints);

		/** Converts a linear space RGB color to an HSV color */
		constexpr LinearColor LinearRGBToHSV() const;

		/** Converts an HSV color to a linear space RGB color */
		constexpr LinearColor HSVToLinearRGB() const;

		/**
		 * Linearly interpolates between two colors by the specified progress amount.  The
		 * interpolation is performed in HSV color space taking the shortest path to the new color's
		 * hue.  This can give better results than Math::Lerp(), but is much more expensive. The
		 * incoming colors are in RGB space, and the output color will be RGB.  The alpha value will
		 * also be interpolated.
		 *
		 * @param	From		The color and alpha to interpolate from as linear RGBA
		 * @param	To			The color and alpha to interpolate to as linear RGBA
		 * @param	Progress	scalar interpolation amount (usually between 0.0 and 1.0 inclusive)
		 * @return	The interpolated color in linear RGB space along with the interpolated alpha
		 * value
		 */
		static LinearColor LerpUsingHSV(
		    const LinearColor& From, const LinearColor& To, const float Progress);

		/** Quantizes the linear color and returns the result as a Color.  This bypasses the SRGB
		 * conversion. */
		Color Quantize() const;

		/** Quantizes the linear color with rounding and returns the result as a Color.  This
		 * bypasses the SRGB conversion. */
		Color QuantizeRound() const;

		/** Quantizes the linear color and returns the result as a Color with optional sRGB
		 * conversion and quality as goal. */
		constexpr Color ToColor(const bool bSRGB = true) const;

		/**
		 * Returns a desaturated color, with 0 meaning no desaturation and 1 == full desaturation
		 *
		 * @param	Desaturation	Desaturation factor in range [0..1]
		 * @return	Desaturated color
		 */
		LinearColor Desaturate(float desaturation) const;
		LinearColor Darken(float delta, bool relative = true) const;
		LinearColor Lighten(float delta, bool relative = true) const;

		LinearColor Translucency(float alpha) const;

		/** Computes the perceptually weighted luminance value of a color. */
		inline float ComputeLuminance() const
		{
			return r * 0.3f + g * 0.59f + b * 0.11f;
		}

		/**
		 * Returns the maximum value in this color structure
		 *
		 * @return The maximum color channel value
		 */
		float GetMax() const
		{
			return Math::Max(Math::Max(Math::Max(r, g), b), a);
		}

		/** useful to detect if a light contribution needs to be rendered */
		bool IsAlmostBlack() const
		{
			return Math::Square(r) < Math::SMALLER_NUMBER &&
			       Math::Square(g) < Math::SMALLER_NUMBER && Math::Square(b) < Math::SMALLER_NUMBER;
		}

		/**
		 * Returns the minimum value in this color structure
		 *
		 * @return The minimum color channel value
		 */
		float GetMin() const
		{
			return Math::Min(Math::Min(Math::Min(r, g), b), a);
		}

		float GetLuminance() const
		{
			return r * 0.3f + g * 0.59f + b * 0.11f;
		}

		String ToString() const
		{
			return Strings::Format(TX("(r={},g={},b={},a={})"), r, g, b, a);
		}

		// Common colors.
		static const LinearColor White;
		static const LinearColor Gray;
		static const LinearColor Black;
		static const LinearColor Transparent;
		static const LinearColor Red;
		static const LinearColor Green;
		static const LinearColor Blue;
		static const LinearColor Yellow;
	};

	inline CORE_API LinearColor operator*(float scalar, const LinearColor& Color)
	{
		return Color.operator*(scalar);
	}


	/**
	 * Color
	 * Stores a color with 8 bits of precision per channel.
	 * Note: Linear color values should always be converted to gamma space before stored in a
	 * Color, as 8 bits of precision is not enough to store linear space colors! This can be done
	 * with FLinearColor::ToColor(true)
	 */
	struct CORE_API Color
	{
	public:
#pragma warning(disable : 4201)    // Avoid warning about nameless struct
		union
		{
			struct
			{
				u8 r, g, b, a;
			};
			u32 alignmentDummy;
		};
#pragma warning(default : 4201)

		u32& DWColor()
		{
			return *((u32*) this);
		}
		const u32& DWColor() const
		{
			return *((u32*) this);
		}

		// Constructors.
		explicit Color()
		{
			// put these into the body for proper ordering with INTEL vs non-INTEL_BYTE_ORDER
			r = g = b = a = 0;
		}

		constexpr Color(u8 r, u8 g, u8 b, u8 a = 255) : r(r), g(g), b(b), a(a) {}

		explicit Color(u32 InColor)
		{
			DWColor() = InColor;
		}

		// Operators.
		bool operator==(const Color& other) const
		{
			return DWColor() == other.DWColor();
		}

		bool operator!=(const Color& other) const
		{
			return DWColor() != other.DWColor();
		}

		void operator+=(const Color& other)
		{
			r = (u8) Math::Min((i32) r + (i32) other.r, 255);
			g = (u8) Math::Min((i32) g + (i32) other.g, 255);
			b = (u8) Math::Min((i32) b + (i32) other.b, 255);
			a = (u8) Math::Min((i32) a + (i32) other.a, 255);
		}

		LinearColor FromRGBE() const;

		/**
		 * Makes a random but quite nice color.
		 */
		static Color MakeRandomColor();

		/**
		 * Makes a color red->green with the passed in scalar (e.g. 0 is red, 1 is green)
		 */
		static Color MakeRedToGreenColorFromScalar(float scalar);

		/**
		 * Converts temperature in Kelvins of a black body radiator to RGB chromaticism.
		 */
		static Color MakeFromColorTemperature(float Temp);

		/**
		 *	@return a new Color based of this color with the new alpha value.
		 *	Usage: const Color& MyColor = ColorList::Green.WithAlpha(128);
		 */
		Color WithAlpha(u8 alpha) const
		{
			return Color(r, g, b, alpha);
		}

		/**
		 * Reinterprets the color as a linear color.
		 *
		 * @return The linear color representation.
		 */
		LinearColor ReinterpretAsLinear() const
		{
			return LinearColor(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
		}

		/**
		 * Converts this color value to a hexadecimal string.
		 *
		 * The format of the string is RRGGBBAA.
		 *
		 * @return Hexadecimal string.
		 * @see FromHex, ToString
		 */
		String ToHex() const
		{
			return Strings::Format(TX("{:02X}{:02X}{:02X}{:02X}"), r, g, b, a);
		}

		static constexpr Color HexRGB(u32 value)
		{
			return {u8(value >> 16), u8(value >> 8), u8(value >> 0)};
		}
		static constexpr Color HexRGBA(u32 value)
		{
			return {u8(value >> 24), u8(value >> 16), u8(value >> 8), u8(value >> 0)};
		}

		/**
		 * Converts this color value to a string.
		 *
		 * @return The string representation.
		 * @see ToHex
		 */
		String ToString() const
		{
			return Strings::Format(TX("(R={},G={},B={},A={})"), r, g, b, a);
		}

		/**
		 * Gets the color in a packed uint32 format packed in the order ARGB.
		 */
		u32 ToPackedARGB() const
		{
			return (a << 24) | (r << 16) | (g << 8) | (b << 0);
		}

		/**
		 * Gets the color in a packed uint32 format packed in the order ABGR.
		 */
		u32 ToPackedABGR() const
		{
			return (a << 24) | (b << 16) | (g << 8) | (r << 0);
		}

		/**
		 * Gets the color in a packed uint32 format packed in the order RGBA.
		 */
		u32 ToPackedRGBA() const
		{
			return (r << 24) | (g << 16) | (g << 8) | (a << 0);
		}

		/**
		 * Gets the color in a packed uint32 format packed in the order BGRA.
		 */
		u32 ToPackedBGRA() const
		{
			return (b << 24) | (g << 16) | (r << 8) | (a << 0);
		}

		Color Desaturate(float desaturation) const;
		Color Darken(float delta, bool relative = true) const;
		Color Lighten(float delta, bool relative = true) const;
		Color Translucency(u8 alpha) const
		{
			return {r, g, b, alpha};
		}


		/** Some pre-initialized colors, useful for debug code */
		static const Color White;
		static const Color Black;
		static const Color Transparent;
		static const Color Red;
		static const Color Green;
		static const Color Blue;
		static const Color Yellow;
		static const Color Cyan;
		static const Color Magenta;
		static const Color Orange;
		static const Color Purple;
		static const Color Turquoise;
		static const Color Silver;
		static const Color Emerald;

	private:
		/**
		 * Please use .ToColor(true) on FLinearColor if you wish to convert from FLinearColor to
		 * Color, with proper sRGB conversion applied.
		 *
		 * Note: Do not implement or make public.  We don't want people needlessly and implicitly
		 * converting between FLinearColor and Color.  It's not a free conversion.
		 */
		explicit Color(const LinearColor& LinearColor);
	};
	REFLECT_NATIVE_TYPE(Color);


	/** Computes a brightness and a fixed point color from a floating point color. */
	extern void ComputeAndFixedColorAndIntensity(
	    const LinearColor& InLinearColor, Color& OutColor, float& OutIntensity);

	/**
	 * Helper struct for a 16 bit 565 color of a DXT1/3/5 block.
	 */
	struct CORE_API FDXTColor565
	{
		/** Blue component, 5 bit. */
		u16 B : 5;

		/** Green component, 6 bit. */
		u16 G : 6;

		/** Red component, 5 bit */
		u16 R : 5;
	};


	/**
	 * Helper struct for a 16 bit 565 color of a DXT1/3/5 block.
	 */
	struct CORE_API FDXTColor16
	{
		union
		{
			/** 565 Color */
			FDXTColor565 Color565;
			/** 16 bit entity representation for easy access. */
			u16 Value;
		};
	};


	/**
	 * Structure encompassing single DXT1 block.
	 */
	struct CORE_API FDXT1
	{
		/** Color 0/1 */
		union
		{
			FDXTColor16 Color[2];
			u32 Colors;
		};
		/** Indices controlling how to blend colors. */
		u32 Indices;
	};


	/**
	 * Structure encompassing single DXT5 block
	 */
	struct CORE_API FDXT5
	{
		/** Alpha component of DXT5 */
		u8 Alpha[8];
		/** DXT1 color component. */
		FDXT1 DXT1;
	};


	template <>
	struct Hash<LinearColor>
	{
		sizet operator()(const LinearColor& color) const
		{
			return HashBytes(&color, sizeof(LinearColor));
		}
	};

	template <>
	struct Hash<Color>
	{
		sizet operator()(const Color& color) const
		{
			return color.DWColor();
		}
	};


	void Read(Serl::ReadContext& ct, LinearColor& color);
	void Write(Serl::WriteContext& ct, const LinearColor& color);

	void Read(Serl::ReadContext& ct, Color& color);
	void Write(Serl::WriteContext& ct, Color color);


	////////////////////////////////////////////////////////
	// Color & LinearColor inline functions

	/**
	 * Helper used by Color -> FLinearColor conversion. We don't use a lookup table as unlike pow,
	 * multiplication is fast.
	 */
	static constexpr float OneOver255 = 1.0f / 255.0f;

	constexpr LinearColor::LinearColor(const Color& other)
	{
		r = float(sRGBToLinearTable[other.r]);
		g = float(sRGBToLinearTable[other.g]);
		b = float(sRGBToLinearTable[other.b]);
		a = float(other.a) * OneOver255;
	}

	constexpr LinearColor LinearColor::LinearRGBToHSV() const
	{
		const float RGBMin   = Math::Min(r, g, b);
		const float RGBMax   = Math::Max(r, g, b);
		const float RGBRange = RGBMax - RGBMin;

		const float hue =
		    (RGBMax == RGBMin ? 0.0f
		        : RGBMax == r ? Math::Mod((((g - b) / RGBRange) * 60.0f) + 360.0f, 360.0f)
		        : RGBMax == g ? (((b - r) / RGBRange) * 60.0f) + 120.0f
		        : RGBMax == b ? (((r - g) / RGBRange) * 60.0f) + 240.0f
		                      : 0.0f);

		const float Saturation = (RGBMax == 0.0f ? 0.0f : RGBRange / RGBMax);
		const float Value      = RGBMax;

		// In the new color, r = H, g = S, b = V, a = A
		return LinearColor(hue, Saturation, Value, a);
	}

	constexpr LinearColor LinearColor::HSVToLinearRGB() const
	{
		// In this color, r = H, g = S, b = V
		const float Hue        = r;
		const float Saturation = g;
		const float Value      = b;

		const float HDiv60          = Hue / 60.0f;
		const float HDiv60_Floor    = Math::Floor(HDiv60);
		const float HDiv60_Fraction = HDiv60 - HDiv60_Floor;

		const float RGBValues[4] = {
		    Value,
		    Value * (1.0f - Saturation),
		    Value * (1.0f - (HDiv60_Fraction * Saturation)),
		    Value * (1.0f - ((1.0f - HDiv60_Fraction) * Saturation)),
		};
		const u32 RGBSwizzle[6][3] = {
		    {0, 3, 1},
		    {2, 0, 1},
		    {1, 0, 3},
		    {1, 2, 0},
		    {3, 1, 0},
		    {0, 1, 2},
		};
		const u32 SwizzleIndex = ((u32) HDiv60_Floor) % 6;

		return LinearColor(RGBValues[RGBSwizzle[SwizzleIndex][0]],
		    RGBValues[RGBSwizzle[SwizzleIndex][1]], RGBValues[RGBSwizzle[SwizzleIndex][2]], a);
	}

	inline LinearColor LinearColor::LerpUsingHSV(
	    const LinearColor& From, const LinearColor& To, const float Progress)
	{
		const LinearColor FromHSV = From.LinearRGBToHSV();
		const LinearColor ToHSV   = To.LinearRGBToHSV();

		float FromHue = FromHSV.r;
		float ToHue   = ToHSV.r;

		// Take the shortest path to the new hue
		if (Math::Abs(FromHue - ToHue) > 180.0f)
		{
			if (ToHue > FromHue)
			{
				FromHue += 360.0f;
			}
			else
			{
				ToHue += 360.0f;
			}
		}

		float NewHue = Math::Lerp(FromHue, ToHue, Progress);

		NewHue = Math::Mod(NewHue, 360.0f);
		if (NewHue < 0.0f)
		{
			NewHue += 360.0f;
		}

		const float NewSaturation = Math::Lerp(FromHSV.g, ToHSV.g, Progress);
		const float NewValue      = Math::Lerp(FromHSV.b, ToHSV.b, Progress);
		LinearColor Interpolated  = LinearColor(NewHue, NewSaturation, NewValue).HSVToLinearRGB();

		const float NewAlpha = Math::Lerp(From.a, To.a, Progress);
		Interpolated.a       = NewAlpha;

		return Interpolated;
	}

	constexpr Color LinearColor::ToColor(const bool bSRGB) const
	{
		float floatr = Math::Clamp(r, 0.0f, 1.0f);
		float floatg = Math::Clamp(g, 0.0f, 1.0f);
		float floatb = Math::Clamp(b, 0.0f, 1.0f);
		float floata = Math::Clamp(a, 0.0f, 1.0f);

		if (bSRGB)
		{
			floatr = floatr <= 0.0031308f ? floatr * 12.92f
			                              : Math::Pow(floatr, 1.0f / 2.4f) * 1.055f - 0.055f;
			floatg = floatg <= 0.0031308f ? floatg * 12.92f
			                              : Math::Pow(floatg, 1.0f / 2.4f) * 1.055f - 0.055f;
			floatb = floatb <= 0.0031308f ? floatb * 12.92f
			                              : Math::Pow(floatb, 1.0f / 2.4f) * 1.055f - 0.055f;
		}

		Color ret;
		ret.r = (u8) Math::FloorToI32(floatr * 255.999f);
		ret.g = (u8) Math::FloorToI32(floatg * 255.999f);
		ret.b = (u8) Math::FloorToI32(floatb * 255.999f);
		ret.a = (u8) Math::FloorToI32(floata * 255.999f);
		return ret;
	}
}    // namespace Rift


// These types act like a POD
RIFT_DECLARE_IS_TRIVIAL(Rift::FDXT1, true);
RIFT_DECLARE_IS_TRIVIAL(Rift::FDXT5, true);
RIFT_DECLARE_IS_TRIVIAL(Rift::FDXTColor16, true);
RIFT_DECLARE_IS_TRIVIAL(Rift::FDXTColor565, true);
