// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Hash.h"

#include <fmt/format.h>

#include <iostream>


namespace p
{
	struct PIPE_API TypeId
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
	inline consteval TypeId GetTypeId()
	{
		return TypeId{p::GetStringHash(TX(UNIQUE_FUNCTION_ID))};
	}


	template<>
	struct Hash<TypeId>
	{
		sizet operator()(const TypeId& id) const
		{
			const Hash<u64> hasher{};
			return hasher(id.GetId());
		}
	};
}    // namespace p


template<>
struct fmt::formatter<p::TypeId> : public fmt::formatter<p::u64>
{
	template<typename FormatContext>
	auto format(const p::TypeId& typeId, FormatContext& ctx)
	{
		return formatter<p::u64>::format(typeId.GetId(), ctx);
	}
};
