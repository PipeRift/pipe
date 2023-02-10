// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Core/Tag.h"
#include "Pipe/Serialize/Serialization.h"

#include <mutex>
#include <shared_mutex>


namespace p
{
	const String TagTable::noneStr{"none"};
	const Tag::Id Tag::noneId{0};

	// Mutex that allows sync reads but waits for registries
	std::shared_mutex editTableMutex{};


	sizet TagTable::Register(StringView str)
	{
		if (str.empty())
		{
			return Tag::noneId;
		}

		static constexpr Hash<StringView> hasher{};

		const sizet hash = hasher(str);
		if (!table.contains({hash}))
		{
			std::unique_lock lock{editTableMutex};
			table.insert({str, hash});
		}
		return hash;
	}

	const String& TagTable::Find(sizet hash) const
	{
		if (hash == Tag::noneId)
		{
			return noneStr;
		}

		// Ensure no other thread is editing the table
		std::shared_lock lock{editTableMutex};
		const ConstIterator foundIt = table.find({hash});
		if (foundIt != table.end()) [[likely]]
		{
			return foundIt->value;
		}
		// Should never reach
		return noneStr;
	}

	TagTable& TagTable::Get()
	{
		static TagTable instance{};
		return instance;
	}

	void Tag::Read(Reader& ct)
	{
		String str;
		ct.Serialize(str);

		*this = Strings::Equals(str, TagTable::noneStr) ? None() : Tag(str);
	}
	void Tag::Write(Writer& ct) const
	{
		ct.Serialize(ToString());
	}
}    // namespace p
