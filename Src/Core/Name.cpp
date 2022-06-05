// Copyright 2015-2022 Piperift - All rights reserved

#include "Core/Name.h"

#include "Serialization/Contexts.h"


namespace pipe
{
	const String NameTable::noneStr{"none"};
	const Name::Id Name::noneId{0};

	sizet NameTable::Register(StringView str)
	{
		if (str.empty())
		{
			return Name::noneId;
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

	const String& NameTable::Find(sizet hash) const
	{
		if (hash == Name::noneId)
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

	NameTable& NameTable::Get()
	{
		static NameTable instance{};
		return instance;
	}

	void Name::Read(serl::ReadContext& ct)
	{
		String str;
		ct.Serialize(str);

		*this = Strings::Equals(str, NameTable::noneStr) ? None() : str;
	}
	void Name::Write(serl::WriteContext& ct) const
	{
		ct.Serialize(ToString());
	}
}    // namespace pipe
