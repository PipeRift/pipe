// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <PCore/Platform.h>
#include <PCore/TypeList.h>
#include <PRefl/Builders/NativeTypeBuilder.h>
#include <PSerl/SerializationFwd.h>


namespace p::ecs
{
	enum class Id : u32
	{};


	template<typename, typename = void>
	struct BaseIdTraits;
	template<typename, typename = void>
	struct IdTraits;

	template<>
	struct BaseIdTraits<u32>
	{
		using Entity     = u32;
		using Index      = u32;
		using Version    = u16;
		using Difference = i64;

		static constexpr Entity indexMask   = 0xfffff;
		static constexpr Entity versionMask = 0xfff;
		static constexpr sizet indexShift   = 20u;
	};

	template<>
	struct BaseIdTraits<u64>
	{
		using Entity     = u64;
		using Index      = u32;
		using Version    = u32;
		using Difference = i64;

		static constexpr Entity indexMask   = 0xffffffff;
		static constexpr Entity versionMask = 0xffffffff;
		static constexpr sizet indexShift   = 32u;
	};

	template<IsEnum T>
	struct IdTraits<T> : public BaseIdTraits<UnderlyingType<T>>
	{
		using Parent     = BaseIdTraits<UnderlyingType<T>>;
		using Entity     = typename Parent::Entity;
		using Index      = typename Parent::Index;
		using Version    = typename Parent::Version;
		using Difference = typename Parent::Difference;
	};

	template<typename T>
		requires(std::is_class_v<T>)
	struct IdTraits<T> : IdTraits<typename T::EntityType>
	{};


	static constexpr Id MakeId(IdTraits<Id>::Index index = IdTraits<Id>::indexMask,
	    IdTraits<Id>::Version version                    = IdTraits<Id>::versionMask)
	{
		return Id{(index & IdTraits<Id>::indexMask)
		          | (IdTraits<Id>::Entity(version) << IdTraits<Id>::indexShift)};
	}

	constexpr Id NoId = MakeId();

	static constexpr IdTraits<Id>::Index GetIndex(Id id)
	{
		return IdTraits<Id>::Index{IdTraits<Id>::Entity(id) & IdTraits<Id>::indexMask};
	}

	static constexpr IdTraits<Id>::Version GetVersion(Id id)
	{
		constexpr auto mask = IdTraits<Id>::versionMask << IdTraits<Id>::indexShift;
		return IdTraits<Id>::Version((IdTraits<Id>::Entity(id) & mask) >> IdTraits<Id>::indexShift);
	}

	constexpr bool IsNone(ecs::Id id)
	{
		return ecs::GetVersion(id) == ecs::GetVersion(ecs::NoId);
	}
}    // namespace p::ecs

REFLECT_NATIVE_TYPE(p::ecs::Id);
