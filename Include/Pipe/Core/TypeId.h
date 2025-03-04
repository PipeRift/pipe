// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Hash.h"
#include "Pipe/Core/Utility.h"
#include "PipePlatform.h"

#if P_DEBUG
	#include "Pipe/Core/StringView.h"
	#include "Pipe/Core/TypeName.h"
#endif

#include <format>
#include <iostream>


namespace p
{
	struct P_API TypeId
	{
	protected:
		u64 id;
#if P_DEBUG
		StringView debugName;
#endif


	public:
		constexpr TypeId() : id{0} {}
		constexpr TypeId(p::Undefined) {}
		explicit constexpr TypeId(u64 id) : id{id} {}
#if P_DEBUG
		explicit constexpr TypeId(u64 id, StringView debugName) : id{id}, debugName{debugName} {}
#endif

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
		constexpr operator bool() const
		{
			return IsValid();
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
	inline consteval TypeId GetTypeId() requires(!IsConst<T>)
	{
		return TypeId{p::GetStringHash(P_UNIQUE_FUNCTION_ID)
#if P_DEBUG
		                  ,
		    GetTypeName<T>()
#endif
		};
	}

	template<typename T>
	inline consteval TypeId GetTypeId() requires(IsConst<T>)
	{
		return GetTypeId<Mut<T>>();
	}


#pragma region Casteable
	struct Casteable
	{
	private:
		mutable TypeId typeId;

	public:
		TypeId GetTypeId() const
		{
			if (!typeId)
			{
				typeId = ProvideTypeId();
			}
			return typeId;
		}

	protected:
		virtual TypeId ProvideTypeId() const = 0;
	};

	template<typename T>
	concept IsCasteable = Derived<std::remove_pointer_t<T>, Casteable, false>;
#pragma endregion Casteable
}    // namespace p


template<>
struct std::formatter<p::TypeId> : public std::formatter<p::u64>
{
	template<typename FormatContext>
	auto format(const p::TypeId& typeId, FormatContext& ctx) const
	{
		return formatter<p::u64>::format(typeId.GetId(), ctx);
	}
};
