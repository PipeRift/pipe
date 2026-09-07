// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Core/TypeTraits.h"
#include "PipePlatform.h"


// Forward declarations of all math types
namespace p
{
#pragma region Vec

	template<u32 size, Number T>
	struct Vec;

	using v2  = Vec<2, float>;
	using v3  = Vec<3, float>;
	using v4  = Vec<4, float>;
	using v2d = Vec<2, double>;
	using v3d = Vec<3, double>;
	using v4d = Vec<4, double>;

	using v2_i8 = Vec<2, i8>;
	using v2_u8 = Vec<2, u8>;
	using v3_i8 = Vec<3, i8>;
	using v3_u8 = Vec<3, u8>;
	using v4_i8 = Vec<4, i8>;
	using v4_u8 = Vec<4, u8>;

	using v2_i32 = Vec<2, i32>;
	using v2_u32 = Vec<2, u32>;
	using v3_i32 = Vec<3, i32>;
	using v3_u32 = Vec<3, u32>;
	using v4_i32 = Vec<4, i32>;
	using v4_u32 = Vec<4, u32>;

	using v2_i64 = Vec<2, i64>;
	using v2_u64 = Vec<2, u64>;
	using v3_i64 = Vec<3, i64>;
	using v3_u64 = Vec<3, u64>;
	using v4_i64 = Vec<4, i64>;
	using v4_u64 = Vec<4, u64>;

#pragma endregion

#pragma region TAABB

	template<u32 size, Number T>
	struct TAABB;

	template<typename Type>
	using TRect = TAABB<2, Type>;
	template<typename Type>
	using TBox = TAABB<3, Type>;

	using Rect  = TRect<float>;
	using Box   = TBox<float>;
	using Recti = TRect<i32>;
	using Boxi  = TBox<i32>;
	using Rectu = TRect<u32>;
	using Boxu  = TBox<u32>;

#pragma endregion

#pragma region Rotator
	struct Rotator;
#pragma endregion

#pragma region Quat
	struct Quat;
#pragma endregion
};    // namespace p
