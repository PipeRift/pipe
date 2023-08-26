// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Core/Tag.h"

#include "Pipe/Memory/Memory.h"
#include "Pipe/Serialize/Serialization.h"

#include <mutex>
#include <shared_mutex>


namespace p::core
{
	struct TagHeader
	{
		u32 activeTags;
		u32 size;

		TagHeader(const TagHeader&) = delete;
		const TChar* Data() const
		{
			return reinterpret_cast<const TChar*>(this + 1);
		};
	};

	TagHeader* GetTagHeader(const TChar* data)
	{
		return reinterpret_cast<TagHeader*>(const_cast<TChar*>(data)) - 1;
	}

	struct TagHeaderRef
	{
		sizet hash     = 0;
		TagHeader* str = nullptr;
	};


	/** Global table storing all tag string data */
	class TagStringTable
	{
		friend Tag;
		struct MultiLinearArena arena;
		TArray<TagHeaderRef> strings;
		bool automaticFlush = true;

	public:
		TagStringTable();
		TagHeader& GetOrAddTagString(sizet hash, StringView value);
		void FreeTagString(sizet hash, TagHeader& str);
	};


	static TagStringTable table{};
	// Makes sure the hashes & keys lists are thread-safe
	std::shared_mutex stringsListMutex;


	Tag::Tag(StringView value)
	{
		if (!value.empty())
		{
			hash              = Hash<StringView>()(value);
			TagHeader& header = table.GetOrAddTagString(hash, value);
			++header.activeTags;
			str = header.Data();
		}
	}

	Tag::Tag(const Tag& other) : hash(other.hash), str(other.str)
	{
		if (str)
		{
			++GetTagHeader(str)->activeTags;
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
			++GetTagHeader(str)->activeTags;
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

	const TChar* Tag::Data() const
	{
		return str;
	}
	u32 Tag::Size() const
	{
		return str ? GetTagHeader(str)->size : 0;
	}
	StringView Tag::AsString() const
	{
		if (str)
		{
			return StringView{str, sizet(GetTagHeader(str)->size)};
		}
		return {};
	}

	constexpr sizet GetAllocSize(sizet dataSize)
	{
		// +1 for the end character of the string
		return sizeof(TagHeader) + sizeof(TChar) * (dataSize + 1);
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
			auto* const header = GetTagHeader(str);
			--header->activeTags;
			if (table.automaticFlush && header->activeTags == 0) [[unlikely]]
			{
				table.FreeTagString(hash, *header);
			}
		}
	}

	bool operator<(const TagHeaderRef& ref, sizet hash)
	{
		return ref.hash < hash;
	}
	bool operator<(sizet hash, const TagHeaderRef& ref)
	{
		return hash < ref.hash;
	}


	TagStringTable::TagStringTable()
	{
		// Ensure pipe memory is initialized. This guaranties memory is deinitialized before tags
		InitializeMemory();
	}

	TagHeader& TagStringTable::GetOrAddTagString(sizet hash, StringView value)
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
		auto* header = static_cast<TagHeader*>(arena.Alloc(GetAllocSize(size), alignof(TagHeader)));
		header->activeTags = 0;
		header->size       = size;
		// Copy string data
		auto* const data = const_cast<TChar*>(header->Data());
		p::CopyMem(data, (void*)value.data(), sizeof(TChar) * size);
		data[header->size] = '\0';

		std::unique_lock lock{stringsListMutex};
		strings.Insert(index, {hash, header});
		return *header;
	}

	void TagStringTable::FreeTagString(sizet hash, TagHeader& str)
	{
		std::unique_lock lock{stringsListMutex};
		strings.RemoveSorted(hash, {}, false);
		arena.Free(&str, GetAllocSize(str.size));
	}

	void Read(Reader& ct, Tag& tag)
	{
		StringView str;
		ct.Serialize(str);
		tag = Tag(str);
	}
	void Write(Writer& ct, const Tag& tag)
	{
		ct.Serialize(tag.AsString());
	}
}    // namespace p::core
