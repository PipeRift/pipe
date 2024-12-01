// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "PipePlatform.h"

#include <limits>


namespace p
{
	using FloatDenormStyle = std::float_denorm_style;
	using FloatRoundStyle  = std::float_round_style;

	template<typename T>
	struct Limits
	{
	private:
		using stdLimits = std::numeric_limits<T>;

	public:
		static constexpr FloatDenormStyle hasDenorm  = stdLimits::has_denorm;
		static constexpr bool hasDenormLoss          = stdLimits::has_denorm_loss;
		static constexpr bool hasInfinity            = stdLimits::has_infinity;
		static constexpr bool hasQuietNaN            = stdLimits::has_quiet_NaN;
		static constexpr bool hasSignalingNaN        = stdLimits::has_signaling_NaN;
		static constexpr bool isBounded              = stdLimits::is_bounded;
		static constexpr bool isExact                = stdLimits::is_exact;
		static constexpr bool isIEC559               = stdLimits::is_iec559;
		static constexpr bool isInteger              = stdLimits::is_integer;
		static constexpr bool isModulo               = stdLimits::is_modulo;
		static constexpr bool isSigned               = stdLimits::is_signed;
		static constexpr bool isSpecialized          = stdLimits::is_specialized;
		static constexpr bool tinynessBefore         = stdLimits::tinyness_before;
		static constexpr bool traps                  = stdLimits::traps;
		static constexpr FloatRoundStyle round_style = stdLimits::round_style;
		static constexpr i32 digits                  = stdLimits::digits;
		static constexpr i32 digits10                = stdLimits::digits10;
		static constexpr i32 maxDigits10             = stdLimits::max_digits10;
		static constexpr i32 maxExponent             = stdLimits::max_exponent;
		static constexpr i32 maxExponent10           = stdLimits::max_exponent10;
		static constexpr i32 minExponent             = stdLimits::min_exponent;
		static constexpr i32 minExponent10           = stdLimits::min_exponent10;
		static constexpr i32 radix                   = stdLimits::radix;


		static constexpr T Min() noexcept
		{
			return stdLimits::min();
		}
		static constexpr T Max() noexcept
		{
			return stdLimits::max();
		}

		static constexpr T Lowest() noexcept
		{
			return stdLimits::lowest();
		}

		static constexpr T Epsilon() noexcept
		{
			return stdLimits::epsilon();
		}

		static constexpr T RoundError() noexcept
		{
			return stdLimits::round_error();
		}

		static constexpr T DenormMin() noexcept
		{
			return stdLimits::deform_min();
		}

		static constexpr T Infinity() noexcept
		{
			return stdLimits::infinity();
		}

		static constexpr T QuietNaN() noexcept
		{
			return stdLimits::quiet_NaN();
		}

		static constexpr T SignalingNaN() noexcept
		{
			return stdLimits::signaling_NaN();
		}
	};
}    // namespace p
