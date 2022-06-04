// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Core/Hash.h"

#include <fmt/format.h>

#include <iostream>


namespace Pipe::Refl
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
		constexpr auto operator<(const TypeId& other) const
		{
			return id < other.id;
		}
		constexpr auto operator>(const TypeId& other) const
		{
			return id > other.id;
		}
		constexpr auto operator<=(const TypeId& other) const
		{
			return id <= other.id;
		}
		constexpr auto operator>=(const TypeId& other) const
		{
			return id >= other.id;
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

	template<typename T>
	inline consteval Refl::TypeId GetTypeId()
	{
		return Refl::TypeId{Pipe::GetStringHash(TX(UNIQUE_FUNCTION_ID))};
	}

	template<>
	struct Hash<Refl::TypeId>
	{
		sizet operator()(const Refl::TypeId& id) const
		{
			const Hash<u64> hasher{};
			return hasher(id.GetId());
		}
	};
}    // namespace Pipe::Refl

namespace Pipe
{
	using namespace Pipe::Refl;
}


template<>
struct fmt::formatter<Pipe::Refl::TypeId> : public fmt::formatter<Pipe::u64>
{
	template<typename FormatContext>
	auto format(const Pipe::Refl::TypeId& typeId, FormatContext& ctx)
	{
		return formatter<Pipe::u64>::format(typeId.GetId(), ctx);
	}
};
