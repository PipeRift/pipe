// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Core/Tag.h"

#include "Pipe/Memory/Memory.h"
#include "Pipe/Serialize/Serialization.h"

#include <mutex>
#include <shared_mutex>


namespace p::core
{
	static TagTable table{};
	// Mutex that allows sync reads but waits for registries
	std::shared_mutex editTableMutex{};


	struct TagKey
	{
		u32 activeTags;
		u32 size;
		const TChar* Data() const
		{
			return reinterpret_cast<const TChar*>(this + 1);
		};
	};


	Tag::Tag(StringView value)
	{
		if (!value.empty())
		{
			hash = Hash<StringView>()(value);
			key  = table.GetOrAddTagKey(hash, value);
			++key->activeTags;
		}
	}

	Tag::Tag(const Tag& other) : hash(other.hash), key(other.key)
	{
		if (key)
		{
			++key->activeTags;
		}
	}

	Tag::Tag(Tag&& other) noexcept
	{
		hash       = other.hash;
		key        = other.key;
		other.hash = 0;
		other.key  = nullptr;
	}
	Tag& Tag::operator=(const Tag& other)
	{
		hash = other.hash;
		key  = other.key;
		if (key)
		{
			++key->activeTags;
		}
		return *this;
	}
	Tag& Tag::operator=(Tag&& other) noexcept
	{
		hash       = other.hash;
		key        = other.key;
		other.hash = 0;
		other.key  = nullptr;
		return *this;
	}
	Tag::~Tag()
	{
		if (key)
		{
			--key->activeTags;
			if (table.automaticFlush && key->activeTags == 0) [[unlikely]]
			{
				table.FreeTagKey(hash, key);
			}
		}
	}

	StringView Tag::AsString() const
	{
		return key ? StringView{key->Data(), sizet(key->size)} : StringView{};
	}

	void Tag::Read(Reader& ct)
	{
		StringView str;
		ct.Serialize(str);
		*this = Tag(str);
	}
	void Tag::Write(Writer& ct) const
	{
		ct.Serialize(AsString());
	}

	constexpr sizet GetKeyAllocSize(sizet dataSize)
	{
		// +1 for the end character of the string
		return sizeof(TagKey) + sizeof(TChar) * (dataSize + 1);
	}

	i32 Tag::FlushInactiveTags()
	{
		const i32 initialSize = table.keys.Size();
		for (i32 i = initialSize - 1; i >= 0; --i)
		{
			auto* key = table.keys[i];
			if (key->activeTags == 0)
			{
				table.arena.Free(key, GetKeyAllocSize(key->size));
				table.hashes.RemoveAt(i, false);
				table.keys.RemoveAt(i, false);
			}
		}
		table.hashes.Shrink();
		table.keys.Shrink();
		return initialSize - table.keys.Size();
	}

	void Tag::SetAutomaticFlush(bool enabled)
	{
		table.automaticFlush = enabled;
	}

	TagKey* TagTable::GetOrAddTagKey(sizet hash, StringView value)
	{
		TagKey* key;
		auto index = hashes.LowerBound(hash);
		if (index != NO_INDEX)
		{
			if (hash == hashes[index])
			{
				// Found existing key
				return keys[index];
			}
		}
		else
		{
			index = hashes.Size();    // Insert last
		}

		const sizet size = value.size();
		key = reinterpret_cast<TagKey*>(p::Alloc(arena, GetKeyAllocSize(size), alignof(TagKey)));
		key->activeTags = 0;
		key->size       = size;

		auto* data = const_cast<TChar*>(key->Data());
		p::CopyMem(data, (void*)value.data(), sizeof(TChar) * size);
		data[key->size] = '\0';

		hashes.Insert(index, hash);
		keys.Insert(index, key);
		return key;
	}

	void TagTable::FreeTagKey(sizet hash, TagKey* key)
	{
		arena.Free(key, GetKeyAllocSize(key->size));
		const i32 index = hashes.FindSortedEqual(hash);
		hashes.RemoveAt(index, false);
		keys.RemoveAt(index, false);
	}
}    // namespace p::core
