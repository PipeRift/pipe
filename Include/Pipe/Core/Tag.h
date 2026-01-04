// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "Pipe/Core/String.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Utility.h"
#include "PipeSerializeFwd.h"

#include <format>


namespace p
{
	/**
	 * An string identified by id.
	 * Searching, comparing and other operations are way cheaper, but creating (indexing) is more
	 * expensive.
	 */
	struct P_API Tag
	{
	private:
		sizet hash      = 0;
		const char* str = nullptr;


	public:
		constexpr Tag() = default;
		Tag(StringView value);
		Tag(const char* value) : Tag(StringView{value}) {}
		explicit Tag(const String& value) : Tag(StringView(value)) {}

		Tag(const Tag& other);
		Tag(Tag&& other) noexcept;
		Tag& operator=(const Tag& other);
		Tag& operator=(Tag&& other) noexcept;
		~Tag();

		const char* Data() const;
		u32 Size() const;
		StringView AsString() const;

		bool IsNone() const
		{
			return str == nullptr;
		}

		bool operator==(const Tag& other) const
		{
			return hash == other.hash;
		}

		bool operator<(const Tag& other) const
		{
			return hash < other.hash;
		}

		sizet GetHash() const
		{
			return hash;
		}

		static const Tag None()
		{
			return {};
		};

		/**
		 * Frees string data not being used by any tag instances
		 * @returns number of freed strings
		 */
		static i32 FlushInactiveTags();

		/** If enabled, a tag's string data will be instantly freed if no other tag is using it.
		 * Manual flush however will be more performant but FlushInactiveTags must be called
		 * periodically to free memory. Automatic flush is ENABLED by default.
		 */
		static void SetAutomaticFlush(bool enabled);

	private:
		void InternalReset();
	};


	inline StringView ToString(const Tag& tag)
	{
		return tag.AsString();
	}

	inline sizet GetHash(const Tag& tag)
	{
		return tag.GetHash();
	}
}    // namespace p


template<>
struct std::formatter<p::Tag> : public std::formatter<p::StringView>
{
	template<typename FormatContext>
	auto format(const p::Tag& name, FormatContext& ctx) const
	{
		const p::StringView nameStr{name.AsString()};
		return formatter<p::StringView>::format(nameStr, ctx);
	}
};
