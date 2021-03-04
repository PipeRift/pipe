// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "CoreTypes.h"


namespace Rift
{
	namespace Refl
	{
		// TODO: Implement property specific flags
		enum class PropertyTags : u16
		{
			None        = 0,
			DetailsEdit = 1 << 0,
			DetailsView = 1 << 1
		};
	}
	enum class ReflectionTags : u16
	{
		None = 0,

		// Class only Tags
		Abstract = 1 << 0,

		// Class & Property Tags
		Transient = 1 << 8,
		SaveGame  = 1 << 9,

		// Property only Tags
		DetailsEdit = 1 << 12,
		DetailsView = 1 << 13
	};

#define Abstract Rift::ReflectionTags::Abstract
#define Transient Rift::ReflectionTags::Transient
#define SaveGame Rift::ReflectionTags::SaveGame
#define DetailsEdit Rift::ReflectionTags::DetailsEdit
#define DetailsView Rift::ReflectionTags::DetailsView


	constexpr u16 operator*(ReflectionTags f)
	{
		return static_cast<u16>(f);
	}

	constexpr ReflectionTags operator|(ReflectionTags lhs, ReflectionTags rhs)
	{
		return static_cast<ReflectionTags>((*lhs) | (*rhs));
	}

	constexpr ReflectionTags operator&(ReflectionTags lhs, ReflectionTags rhs)
	{
		return static_cast<ReflectionTags>((*lhs) & (*rhs));
	}

	constexpr bool operator!(ReflectionTags lhs)
	{
		return *lhs <= 0;
	}

	constexpr bool operator>(ReflectionTags lhs, u16 rhs)
	{
		return (*lhs) > rhs;
	}

	constexpr bool operator<(ReflectionTags lhs, u16 rhs)
	{
		return (*lhs) < rhs;
	}

	constexpr bool operator>(u16 lhs, ReflectionTags rhs)
	{
		return lhs > (*rhs);
	}

	constexpr bool operator<(u16 lhs, ReflectionTags rhs)
	{
		return lhs < (*rhs);
	}


	// Allows initialization from macros being <> a value of 0
	template <ReflectionTags tags = static_cast<ReflectionTags>(0)>
	struct ReflectionTagsInitializer
	{
		static constexpr ReflectionTags value = tags;
	};
}    // namespace Rift
