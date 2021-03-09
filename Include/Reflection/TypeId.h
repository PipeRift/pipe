// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Misc/Hash.h"


#undef UNIQUE_FUNCTION_ID
#if defined(_MSC_VER)
#	define UNIQUE_FUNCTION_ID __FUNCSIG__
#else
#	if defined(__GNUG__)
#		define UNIQUE_FUNCTION_ID __PRETTY_FUNCTION__
#	endif
#endif


namespace Rift
{
	namespace Refl
	{
		struct CORE_API TypeId
		{
		protected:
			u64 id = 0;


		public:
			constexpr TypeId() = default;
			explicit constexpr TypeId(u64 id) : id(id) {}

			template <typename T>
			static constexpr TypeId Get()
			{
				return TypeId{Rift::GetStringHash(TX(UNIQUE_FUNCTION_ID))};
			}

			constexpr u64 GetId() const
			{
				return id;
			}

			bool operator==(const TypeId& other) const
			{
				return id == other.id;
			}
		};
	}    // namespace Refl

	template <>
	struct Hash<Refl::TypeId>
	{
		sizet operator()(const Refl::TypeId& id) const
		{
			const Hash<u64> hasher{};
			return hasher(id.GetId());
		}
	};
}    // namespace Rift
