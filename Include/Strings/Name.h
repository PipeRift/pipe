// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Misc/Hash.h"
#include "Reflection/ClassTraits.h"
#include "String.h"

#include <tsl/robin_set.h>

#include <mutex>
#include <shared_mutex>


namespace Rift
{
	struct Name;

	/** Represents an string with an already hashed value */
	struct NameKey
	{
	private:
		static constexpr Hash<String> hasher{};

		String str;
		size_t hash = 0;

	public:
		NameKey() = default;
		NameKey(size_t hash) : hash{hash} {}
		NameKey(StringView inStr) : str{inStr}, hash{hasher(str)} {}

		NameKey(const NameKey& other) : hash{other.hash} {}
		NameKey(NameKey&& other) noexcept : str{Move(other.str)}, hash{other.hash} {}
		NameKey& operator=(const NameKey& other)
		{
			hash = other.hash;
			return *this;
		}

		const String& GetString() const
		{
			return str;
		}
		const size_t GetHash() const
		{
			return hash;
		}

		bool operator==(const NameKey& other) const
		{
			return hash == other.hash;
		}
	};

	template <>
	struct Hash<NameKey>
	{
		size_t operator()(const NameKey& x) const
		{
			return x.GetHash();
		}
	};


	/** Global table storing all names */
	class NameTable
	{
		friend Name;

		// #TODO: Move to TSet
		using Container = tsl::robin_set<NameKey, Hash<NameKey>, std::equal_to<NameKey>>;
		using Iterator = Container::iterator;
		using ConstIterator = Container::const_iterator;

		Container table{};
		// Mutex that allows sync reads but waits for registries
		mutable std::shared_mutex editTableMutex;


		NameTable() = default;

		size_t Register(StringView string);
		const String& Find(size_t hash) const;

		static NameTable& Get()
		{
			static NameTable instance{};
			return instance;
		}
	};


	/**
	 * An string identified by id.
	 * Searching, comparing and other operations are way cheaper, but creating (indexing) is more
	 * expensive.
	 */
	struct CORE_API Name
	{
		friend NameTable;
		using Id = size_t;

	private:
		static const Id noneId;
		static const String noneStr;
		Id id = noneId;
#if BUILD_DEBUG
		StringView value;	 // Only used for debugging purposes
#endif


	public:
		Name() = default;
		Name(const TCHAR* key) : Name(StringView{key}) {}
		Name(StringView key)
		{
			// Index this name
			id = NameTable::Get().Register(key);
#if BUILD_DEBUG
			value = key;
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
			return IsNone() ? noneStr : NameTable::Get().Find(id);
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
			return noneStr;
		}

		bool Serialize(class Archive& ar, const char* name);

	private:
		Name(const Id& id) : id(id) {}
	};

	DEFINE_CLASS_TRAITS(Name, HasCustomSerialize = true);

	template <>
	struct Hash<Name>
	{
		size_t operator()(const Name& k) const
		{
			return k.GetId();
		}
	};

}	 // namespace Rift
