// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Hash.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Memory/MultiLinearArena.h"
#include "Pipe/Reflect/TypeFlags.h"
#include "Pipe/Serialize/SerializationFwd.h"
#include "String.h"

#include <tsl/robin_set.h>


namespace p::core
{
	struct TagKey;


	/**
	 * An string identified by id.
	 * Searching, comparing and other operations are way cheaper, but creating (indexing) is more
	 * expensive.
	 */
	struct PIPE_API Tag
	{
	private:
		sizet hash  = 0;
		TagKey* key = nullptr;


	public:
		constexpr Tag() = default;
		explicit Tag(StringView value);
		Tag(const TChar* key) : Tag(StringView{key}) {}
		explicit Tag(const String& str) : Tag(StringView(str)) {}

		Tag(const Tag& other);
		Tag(Tag&& other) noexcept;
		Tag& operator=(const Tag& other);
		Tag& operator=(Tag&& other) noexcept;
		~Tag();

		StringView AsString() const;

		bool operator==(const Tag& other) const
		{
			return hash == other.hash;
		}

		bool operator<(const Tag& other) const
		{
			return hash < other.hash;
		}

		bool IsNone() const
		{
			return key == nullptr;
		}

		sizet GetHash() const
		{
			return hash;
		}

		static const Tag None()
		{
			return {};
		};

		void Read(Reader& ct);
		void Write(Writer& ct) const;


		/**
		 * Frees string data not being used by any tag instances
		 * @returns number of freed strings
		 */
		static i32 FlushInactiveTags();

		/** If enabled, A tag's string data will be instantly freed if no other tag is using it.
		 * Manual flush will be more performant but FlushInactiveTags must be called periodically to
		 * free memory. Automatic flush is ENABLED by default.
		 */
		static void SetAutomaticFlush(bool enabled);
	};


	inline StringView ToString(const Tag& name)
	{
		return name.AsString();
	}


	/** Global table storing all tag string data */
	class TagTable
	{
		friend Tag;
		struct MultiLinearArena arena;
		TArray<sizet> hashes;
		TArray<TagKey*> keys;
		bool automaticFlush = true;

	public:
		TagKey* GetOrAddTagKey(sizet hash, StringView value);
		void FreeTagKey(sizet hash, TagKey* key);
	};
}    // namespace p::core

namespace p
{
	using namespace p::core;


	template<>
	struct Hash<Tag>
	{
		sizet operator()(const Tag& k) const
		{
			return k.GetHash();
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
		const p::StringView nameStr{name.AsString()};
		return formatter<p::StringView>::format(nameStr, ctx);
	}
};
