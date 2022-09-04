
// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/ECS/Context.h"
#include "Pipe/Serialize/Serialization.h"


namespace p::ecs
{
	class PIPE_API EntityReader : public Reader
	{
		Context& context;

		// While serializing we create ids as Ids appear and link them.
		TArray<Id> ids;


	public:
		EntityReader(const p::Reader& parent, Context& context) : Reader(parent), context{context}
		{}

		void SerializeEntities(p::TArray<Id>& entities, TFunction<void(EntityReader&)> onReadPools);

		void SerializeEntity(Id& entity, TFunction<void(EntityReader&)> onReadPools)
		{
			TArray<Id> entities{entity};
			SerializeEntities(entities, onReadPools);
			entity = entities.IsEmpty() ? NoId : entities[0];
		}

		template<typename T>
		void SerializeComponent();

		template<typename... T>
		void SerializeComponents()
		{
			(SerializeComponent<T>(), ...);
		}

		const TArray<Id>& GetIds() const
		{
			return ids;
		}

		Context& GetContext()
		{
			return context;
		}
	};

	class PIPE_API EntityWriter : public Writer
	{
		Context& context;
		bool includeChildren;

		// While serializing we create ids as Ids appear and link them.
		TArray<Id> ids;
		TMap<Id, i32> idToIndexes;


	public:
		EntityWriter(const Writer& parent, Context& context, bool includeChildren = true)
		    : Writer(parent), context{context}, includeChildren{includeChildren}
		{}

		void SerializeEntities(const TArray<Id>& entities,
		    TFunction<void(EntityWriter&)> onWritePools, bool includeChildren = true);

		void SerializeEntity(
		    Id entity, TFunction<void(EntityWriter&)> onWritePools, bool includeChildren = true)
		{
			SerializeEntities({entity}, onWritePools, includeChildren);
		}

		template<typename T>
		void SerializeComponent();

		template<typename... T>
		void SerializeComponents()
		{
			(SerializeComponent<T>(), ...);
		}

		const TArray<Id>& GetIds() const
		{
			return ids;
		}

		const TMap<Id, i32>& GetIdToIndexes() const
		{
			return idToIndexes;
		}

	private:
		void RetrieveHierarchy(const TArray<Id>& roots, TArray<Id>& children);
		void RemoveIgnoredEntities(TArray<Id>& entities);
	};

	template<typename T>
	inline void EntityReader::SerializeComponent()
	{
		if (EnterNext(GetTypeName<T>(false)))
		{
			auto& pool = GetContext().AssurePool<T>();

			String key;
			BeginObject();
			for (i32 i = 0; i < ids.Size(); ++i)
			{
				const Id node = ids[i];
				key.clear();
				Strings::FormatTo(key, "{}", i);

				if (EnterNext(key))
				{
					if constexpr (!IsEmpty<T>)
					{
						T& comp = pool.GetOrAdd(node);
						Serialize(comp);
					}
					else
					{
						pool.Add(node);
					}
					Leave();
				}
			}
			Leave();
		}
	}

	template<typename T>
	inline void EntityWriter::SerializeComponent()
	{
		TArray<TPair<i32, Id>> componentIds;    // TODO: Make sure this is needed

		auto* pool = context.GetPool<const T>();
		if (pool)
		{
			componentIds.Reserve(math::Min(i32(pool->Size()), ids.Size()));
			for (i32 i = 0; i < ids.Size(); ++i)
			{
				const Id id = ids[i];
				if (pool->Has(id))
				{
					componentIds.Add({i, id});
				}
			}
		}

		if (componentIds.IsEmpty())
		{
			return;
		}

		// FIX: yyjson doesn't seem to take into account stringview length when generating text
		// Temporarely fixed by caching component name keys
		PushAddFlags(p::WriteFlags_CacheStringKeys);
		if (EnterNext(GetTypeName<T>(false)))
		{
			String key;
			BeginObject();
			for (auto id : componentIds)
			{
				key.clear();
				Strings::FormatTo(key, "{}", id.first);

				if constexpr (std::is_empty_v<T>)
				{
					Next(StringView{key}, T{});
				}
				else
				{
					Next(StringView{key}, pool->Get(id.second));
				}
			}
			Leave();
		}
		PopFlags();
	}
}    // namespace p::ecs
