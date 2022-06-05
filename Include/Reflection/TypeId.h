// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Core/Hash.h"

#include <fmt/format.h>

#include <iostream>


namespace pipe::refl
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
	inline consteval refl::TypeId GetTypeId()
	{
		return refl::TypeId{pipe::GetStringHash(TX(UNIQUE_FUNCTION_ID))};
	}
}    // namespace pipe::refl

namespace pipe
{
	using namespace pipe::refl;

	template<>
	struct Hash<refl::TypeId>
	{
		sizet operator()(const refl::TypeId& id) const
		{
			const Hash<u64> hasher{};
			return hasher(id.GetId());
		}
	};
}    // namespace pipe


template<>
struct fmt::formatter<pipe::refl::TypeId> : public fmt::formatter<pipe::u64>
{
	template<typename FormatContext>
	auto format(const pipe::refl::TypeId& typeId, FormatContext& ctx)
	{
		return formatter<pipe::u64>::format(typeId.GetId(), ctx);
	}
};
