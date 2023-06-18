// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/Span.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Reflect/Builders/NativeTypeBuilder.h"
#include "Pipe/Serialize/SerializationFwd.h"


namespace p::ecs
{
	// DECLARATIONS

	/** An Id is an integer composed of both index and version that identifies an entity */
	enum class Id : u32
	{
	};
	constexpr bool IsNone(ecs::Id id);
	void PIPE_API Read(p::Reader& ct, p::ecs::Id& val);
	void PIPE_API Write(p::Writer& ct, p::ecs::Id val);


	/** IdTraits define properties of an Id type based on its size */
	template<typename Type>
	struct IdTraits;
	template<>
	struct IdTraits<u32>
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
	struct IdTraits<u64>
	{
		using Entity     = u64;
		using Index      = u32;
		using Version    = u32;
		using Difference = i64;

		static constexpr Entity indexMask   = 0xffffffff;
		static constexpr Entity versionMask = 0xffffffff;
		static constexpr sizet indexShift   = 32u;
	};
	template<>
	struct IdTraits<Id> : public IdTraits<UnderlyingType<Id>>
	{};


	/** IdRegistry tracks the existance and versioning of ids. Used internally by the ECS context */
	struct PIPE_API IdRegistry
	{
		using Traits  = IdTraits<Id>;
		using Index   = Traits::Index;
		using Version = Traits::Version;

	private:

		TArray<Id> entities;
		TArray<Index> available;


	public:

		IdRegistry() {}
		IdRegistry(IdRegistry&& other)                 = default;
		IdRegistry(const IdRegistry& other)            = default;
		IdRegistry& operator=(IdRegistry&& other)      = default;
		IdRegistry& operator=(const IdRegistry& other) = default;


		Id Create();
		void Create(TSpan<Id> newIds);
		bool Destroy(Id id);
		bool Destroy(TSpan<const Id> ids);
		bool IsValid(Id id) const;

		u32 Size() const
		{
			return entities.Size() - available.Size();
		}

		template<typename Callback>
		void Each(Callback cb) const;
	};


	// DEFINITIONS

	// Creates an id from a combination of index and version. This does NOT create an entity.
	constexpr Id MakeId(IdTraits<Id>::Index index = IdTraits<Id>::indexMask,
	    IdTraits<Id>::Version version             = IdTraits<Id>::versionMask)
	{
		return Id{(index & IdTraits<Id>::indexMask)
		          | (IdTraits<Id>::Entity(version) << IdTraits<Id>::indexShift)};
	}

	// Extract the index from an id
	constexpr IdTraits<Id>::Index GetIndex(Id id)
	{
		return IdTraits<Id>::Index{IdTraits<Id>::Entity(id) & IdTraits<Id>::indexMask};
	}

	// Extract the version from an id
	constexpr IdTraits<Id>::Version GetVersion(Id id)
	{
		constexpr auto mask = IdTraits<Id>::versionMask << IdTraits<Id>::indexShift;
		return IdTraits<Id>::Version((IdTraits<Id>::Entity(id) & mask) >> IdTraits<Id>::indexShift);
	}

	// Invalid value of an Id
	constexpr Id NoId                         = MakeId();
	constexpr IdTraits<Id>::Version NoVersion = GetVersion(NoId);
	constexpr IdTraits<Id>::Index NoIndex     = GetIndex(NoId);

	// Check if an Id contains an invalid version. Use IdRegistry::IsValid to check if it exists
	constexpr bool IsNone(ecs::Id id)
	{
		return ecs::GetVersion(id) == ecs::GetVersion(ecs::NoId);
	}

	template<typename Callback>
	void IdRegistry::Each(Callback cb) const
	{
		if (available.IsEmpty())
		{
			for (i32 i = 0; i < entities.Size(); ++i)
			{
				cb(entities[i]);
			}
		}
		else
		{
			for (i32 i = 0; i < entities.Size(); ++i)
			{
				const Id id = entities[i];
				if (GetIndex(id) == i)
				{
					cb(id);
				}
			}
		}
	}
}    // namespace p::ecs

REFLECT_NATIVE_TYPE(p::ecs::Id);
