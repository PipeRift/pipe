// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Hash.h"
#include "Pipe/Core/Platform.h"

#include <format>
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

		constexpr bool IsValid() const
		{
			return id != 0;
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

	inline sizet GetHash(const TypeId& id)
	{
		return GetHash(id.GetId());
	}

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

}    // namespace p


template<>
struct std::formatter<p::TypeId> : public std::formatter<p::u64>
{
	template<typename FormatContext>
	auto format(const p::TypeId& typeId, FormatContext& ctx)
	{
		return formatter<p::u64>::format(typeId.GetId(), ctx);
	}
};
