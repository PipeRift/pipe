// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Hash.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Reflect/TypeFlags.h"
#include "Pipe/Serialize/SerializationFwd.h"
#include "String.h"

#include <tsl/robin_set.h>


namespace p::core
{
	struct Tag;

	/** Represents an string with an already hashed value */
	struct TagKey
	{
		String value;
		sizet hash = 0;


		TagKey(sizet hash) : hash{hash} {}
		TagKey(StringView value, sizet hash) : value{value}, hash{hash} {}

		TagKey(TagKey&& other) noexcept : value{Move(other.value)}, hash{other.hash} {}
		TagKey(const TagKey& other) = delete;
		TagKey& operator=(TagKey&& other) noexcept
		{
			value      = Move(other.value);
			hash       = other.hash;
			other.hash = 0;
			return *this;
		}
		TagKey& operator=(const TagKey& other) = delete;

		bool operator==(const TagKey& other) const
		{
			return hash == other.hash;
		}
	};
}    // namespace p::core

namespace p
{
	using namespace p::core;


	template<>
	struct Hash<TagKey>
	{
		sizet operator()(const TagKey& x) const
		{
			return x.hash;
		}
	};
}    // namespace p

namespace p::core
{
	/** Global table storing all names */
	class TagTable
	{
		friend Tag;
		static const String noneStr;

		// #TODO: Move to TSet
		using Container     = tsl::robin_set<TagKey, Hash<TagKey>, std::equal_to<TagKey>>;
		using Iterator      = Container::iterator;
		using ConstIterator = Container::const_iterator;

		Container table{};


		TagTable() = default;

		sizet Register(StringView string);
		const String& Find(sizet hash) const;

		static TagTable& Get();
	};


	/**
	 * An string identified by id.
	 * Searching, comparing and other operations are way cheaper, but creating (indexing) is more
	 * expensive.
	 */
	struct PIPE_API Tag
	{
		friend TagTable;
		using Id = sizet;

	private:
		static const Id noneId;
		Id id = noneId;
#if P_DEBUG

#	pragma warning(push)
#	pragma warning(disable:4251)
		StringView value;    // Only used for debugging purposes
#	pragma warning(pop)
#endif


	public:
		Tag() = default;
		Tag(const TChar* key) : Tag(StringView{key}) {}
		explicit Tag(StringView key)
		{
			// Index this name
			id = TagTable::Get().Register(key);
#if P_DEBUG
			value = ToString();
#endif
		}
		explicit Tag(const String& str) : Tag(StringView(str)) {}
		Tag(const Tag& other)
		    : id(other.id)
#if P_DEBUG
		    , value(other.value)
#endif
		{}
		Tag(Tag&& other) noexcept
		{
			std::swap(id, other.id);
#if P_DEBUG
			std::swap(value, other.value);
#endif
		}
		Tag& operator=(const Tag& other) = default;

		Tag& operator=(Tag&& other) noexcept
		{
			std::swap(id, other.id);
#if P_DEBUG
			std::swap(value, other.value);
#endif
			return *this;
		}

		const String& ToString() const
		{
			return TagTable::Get().Find(id);
		}

		bool operator==(const Tag& other) const
		{
			return id == other.id;
		}

		bool operator<(const Tag& other) const
		{
			return id < other.id;
		}

		bool IsNone() const
		{
			return id == noneId;
		}

		const Id& GetId() const
		{
			return id;
		}


		static const Tag& None()
		{
			static Tag none{noneId};
			return none;
		};

		static const String& NoneStr()
		{
			return TagTable::noneStr;
		}

		void Read(Reader& ct);
		void Write(Writer& ct) const;

	private:
		Tag(const Id& id) : id(id) {}
	};


	inline String ToString(const Tag& name)
	{
		return name.ToString();
	}
}    // namespace p::core

namespace p
{
	using namespace p::core;


	template<>
	struct Hash<Tag>
	{
		sizet operator()(const Tag& k) const
		{
			return k.GetId();
		}
	};

	template<>
	struct TFlags<Tag> : public DefaultTFlags
	{
		enum
		{
			HasMemberSerialize = true
		};
	};

	OVERRIDE_TYPE_NAME(Tag)
}    // namespace p


template<>
struct fmt::formatter<p::Tag> : public fmt::formatter<p::StringView>
{
	template<typename FormatContext>
	auto format(const p::Tag& name, FormatContext& ctx)
	{
		const p::StringView nameStr{name.ToString()};
		return formatter<p::StringView>::format(nameStr, ctx);
	}
};
