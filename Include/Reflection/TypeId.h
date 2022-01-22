// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Misc/Hash.h"

#include <compare>
#include <iostream>


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

			constexpr u64 GetId() const
			{
				return id;
			}

			constexpr auto operator==(const TypeId& other) const
			{
				return id == other.id;
			}
			constexpr auto operator<=>(const TypeId& other) const
			{
				return id <=> other.id;
			}

			template<typename T>
			static consteval TypeId Get()
			{
				return TypeId{Rift::GetStringHash(TX(UNIQUE_FUNCTION_ID))};
			}

			static consteval TypeId None()
			{
				return TypeId{};
			}
		};

		inline std::ostream& operator<<(std::ostream& stream, TypeId typeId)
		{
			stream << "TypeId(id=" << typeId.GetId() << ")";
			return stream;
		}
	}    // namespace Refl

	template<>
	struct Hash<Refl::TypeId>
	{
		sizet operator()(const Refl::TypeId& id) const
		{
			const Hash<u64> hasher{};
			return hasher(id.GetId());
		}
	};
}    // namespace Rift
