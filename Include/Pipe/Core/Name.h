// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Hash.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Reflect/TypeFlags.h"
#include "Pipe/Serialize/SerializationFwd.h"
#include "String.h"

#include <tsl/robin_set.h>


namespace p::core
{
	struct Name;

	/** Represents an string with an already hashed value */
	struct NameKey
	{
		String value;
		sizet hash = 0;


		NameKey(sizet hash) : hash{hash} {}
		NameKey(StringView value, sizet hash) : value{value}, hash{hash} {}

		NameKey(NameKey&& other) noexcept : value{Move(other.value)}, hash{other.hash} {}
		NameKey(const NameKey& other) = delete;
		NameKey& operator             =(NameKey&& other) noexcept
		{
			value      = Move(other.value);
			hash       = other.hash;
			other.hash = 0;
			return *this;
		}
		NameKey& operator=(const NameKey& other) = delete;

		bool operator==(const NameKey& other) const
		{
			return hash == other.hash;
		}
	};
}    // namespace p::core

namespace p
{
	using namespace p::core;


	template<>
	struct Hash<NameKey>
	{
		sizet operator()(const NameKey& x) const
		{
			return x.hash;
		}
	};
}    // namespace p

namespace p::core
{
	/** Global table storing all names */
	class NameTable
	{
		friend Name;
		static const String noneStr;


		// #TODO: Move to TSet
		using Container     = tsl::robin_set<NameKey, Hash<NameKey>, std::equal_to<NameKey>>;
		using Iterator      = Container::iterator;
		using ConstIterator = Container::const_iterator;

		Container table{};
		// Mutex that allows sync reads but waits for registries
		mutable class std::shared_mutex editTableMutex;


		NameTable() = default;

		sizet Register(StringView string);
		const String& Find(sizet hash) const;

		static NameTable& Get();
	};


	/**
	 * An string identified by id.
	 * Searching, comparing and other operations are way cheaper, but creating (indexing) is more
	 * expensive.
	 */
	struct PIPE_API Name
	{
		friend NameTable;
		using Id = sizet;

	private:
		static const Id noneId;
		Id id = noneId;
#if BUILD_DEBUG

#	pragma warning(push)
#	pragma warning(disable:4251)
		StringView value;    // Only used for debugging purposes
#	pragma warning(pop)
#endif


	public:
		Name() = default;
		Name(const TChar* key) : Name(StringView{key}) {}
		Name(StringView key)
		{
			// Index this name
			id = NameTable::Get().Register(key);
#if BUILD_DEBUG
			value = ToString();
#endif
		}
		Name(const String& str) : Name(StringView(str)) {}
		Name(const Name& other)
		    : id(other.id)
#if BUILD_DEBUG
		    , value(other.value)
#endif
		{}
		Name(Name&& other) noexcept
		{
			std::swap(id, other.id);
#if BUILD_DEBUG
			std::swap(value, other.value);
#endif
		}
		Name& operator=(const Name& other) = default;

		Name& operator=(Name&& other) noexcept
		{
			std::swap(id, other.id);
#if BUILD_DEBUG
			std::swap(value, other.value);
#endif
			return *this;
		}

		const String& ToString() const
		{
			return NameTable::Get().Find(id);
		}

		bool operator==(const Name& other) const
		{
			return id == other.id;
		}

		bool IsNone() const
		{
			return id == noneId;
		}

		const Id& GetId() const
		{
			return id;
		}


		static const Name& None()
		{
			static Name none{noneId};
			return none;
		};

		static const String& NoneStr()
		{
			return NameTable::noneStr;
		}

		void Read(Reader& ct);
		void Write(Writer& ct) const;

	private:
		Name(const Id& id) : id(id) {}
	};

}    // namespace p::core

namespace p
{
	using namespace p::core;


	template<>
	struct Hash<Name>
	{
		sizet operator()(const Name& k) const
		{
			return k.GetId();
		}
	};

	template<>
	struct TFlags<Name> : public DefaultTFlags
	{
		enum
		{
			HasMemberSerialize = true
		};
	};

	OVERRIDE_TYPE_NAME(Name)
}    // namespace p


template<>
struct fmt::formatter<p::Name> : public fmt::formatter<p::StringView>
{
	template<typename FormatContext>
	auto format(const p::Name& name, FormatContext& ctx)
	{
		const p::StringView nameStr{name.ToString()};
		return formatter<p::StringView>::format(nameStr, ctx);
	}
};
