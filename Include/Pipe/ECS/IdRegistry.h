// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/ECS/Id.h"

#include <Pipe/Core/Array.h>
#include <Pipe/Core/Span.h>


namespace p::ecs
{
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
		IdRegistry(IdRegistry&& other)      = default;
		IdRegistry(const IdRegistry& other) = default;
		IdRegistry& operator=(IdRegistry&& other) = default;
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
		void Each(Callback cb) const
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
	};
}    // namespace p::ecs