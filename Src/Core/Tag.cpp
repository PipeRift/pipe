// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Core/Tag.h"

#include "Pipe/Memory/Memory.h"
#include "Pipe/Serialize/Serialization.h"

#include <mutex>
#include <shared_mutex>


namespace p::core
{
	struct TagString
	{
		u32 activeTags;
		u32 size;

		TagString(const TagString&) = delete;
		const TChar* Data() const
		{
			return reinterpret_cast<const TChar*>(this + 1);
		};
	};

	struct TagStringRef
	{
		sizet hash     = 0;
		TagString* str = nullptr;
	};


	/** Global table storing all tag string data */
	class TagStringTable
	{
		friend Tag;
		struct MultiLinearArena arena;
		TArray<TagStringRef> strings;
		bool automaticFlush = true;

	public:
		TagString& GetOrAddTagString(sizet hash, StringView value);
		void FreeTagString(sizet hash, TagString& str);
	};


	static TagStringTable table{};
	// Makes sure the hashes & keys lists are thread-safe
	std::shared_mutex stringsListMutex;


	Tag::Tag(StringView value)
	{
		if (!value.empty())
		{
			hash = Hash<StringView>()(value);
			str  = &table.GetOrAddTagString(hash, value);
			++str->activeTags;
		}
	}

	Tag::Tag(const Tag& other) : hash(other.hash), str(other.str)
	{
		if (str)
		{
			++str->activeTags;
		}
	}

	Tag::Tag(Tag&& other) noexcept
	{
		hash       = other.hash;
		str        = other.str;
		other.hash = 0;
		other.str  = nullptr;
	}
	Tag& Tag::operator=(const Tag& other)
	{
		if (this == &other)
			return *this;

		InternalReset();
		hash = other.hash;
		str  = other.str;
		if (str)
		{
			++str->activeTags;
		}
		return *this;
	}
	Tag& Tag::operator=(Tag&& other) noexcept
	{
		if (this == &other)
			return *this;

		InternalReset();
		hash       = other.hash;
		str        = other.str;
		other.hash = 0;
		other.str  = nullptr;
		return *this;
	}
	Tag::~Tag()
	{
		InternalReset();
	}

	StringView Tag::AsString() const
	{
		return str ? StringView{str->Data(), sizet(str->size)} : StringView{};
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

	constexpr sizet GetAllocSize(sizet dataSize)
	{
		// +1 for the end character of the string
		return sizeof(TagString) + sizeof(TChar) * (dataSize + 1);
	}

	i32 Tag::FlushInactiveTags()
	{
		std::unique_lock lock{stringsListMutex};
		const i32 initialSize = table.strings.Size();
		for (i32 i = initialSize - 1; i >= 0; --i)
		{
			auto* str = table.strings[i].str;
			if (str->activeTags == 0)
			{
				table.arena.Free(str, GetAllocSize(str->size));
				table.strings.RemoveAt(i, false);
			}
		}
		table.strings.Shrink();
		return initialSize - table.strings.Size();
	}

	void Tag::SetAutomaticFlush(bool enabled)
	{
		table.automaticFlush = enabled;
	}

	void Tag::InternalReset()
	{
		if (str)
		{
			--str->activeTags;
			if (table.automaticFlush && str->activeTags == 0) [[unlikely]]
			{
				table.FreeTagString(hash, *str);
			}
		}
	}

	bool operator<(const TagStringRef& ref, sizet hash)
	{
		return ref.hash < hash;
	}
	bool operator<(sizet hash, const TagStringRef& ref)
	{
		return hash < ref.hash;
	}

	TagString& TagStringTable::GetOrAddTagString(sizet hash, StringView value)
	{
		i32 index;
		{
			std::shared_lock lock{stringsListMutex};
			index = strings.LowerBound(hash);
			if (index != NO_INDEX)
			{
				const auto& ref = strings[index];
				if (hash == ref.hash)
				{
					// Found existing key
					return *ref.str;
				}
			}
			else
			{
				index = strings.Size();    // Insert at the end of the list
			}
		}

		const sizet size = value.size();
		auto* key =
		    static_cast<TagString*>(p::Alloc(arena, GetAllocSize(size), alignof(TagString)));
		key->activeTags = 0;
		key->size       = size;
		// Copy string data
		auto* const data = const_cast<TChar*>(key->Data());
		p::CopyMem(data, (void*)value.data(), sizeof(TChar) * size);
		data[key->size] = '\0';

		std::unique_lock lock{stringsListMutex};
		strings.Insert(index, {hash, key});
		return *key;
	}

	void TagStringTable::FreeTagString(sizet hash, TagString& str)
	{
		std::unique_lock lock{stringsListMutex};
		strings.RemoveSorted(hash, {}, false);
		arena.Free(&str, GetAllocSize(str.size));
	}
}    // namespace p::core
