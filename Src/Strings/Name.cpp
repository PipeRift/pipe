// Copyright 2015-2021 Piperift - All rights reserved

#include "Strings/Name.h"

#include "Serialization/Archive.h"


namespace Rift
{
	const String Name::noneStr{"none"};
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
		// Ensure no other thread is editing the table
		std::shared_lock lock{editTableMutex};
		const ConstIterator foundIt = table.find({hash});
		if (foundIt != table.end())
		{
			return foundIt->value;
		}
		// Should never reach
		return Name::NoneStr();
	}

	NameTable& NameTable::Get()
	{
		static NameTable instance{};
		return instance;
	}

	bool Name::Serialize(Archive& ar, StringView name)
	{
		if (ar.IsSaving())
		{
			String str = ToString();
			ar(name, str);
		}
		else
		{
			String str;
			ar(name, str);

			if (CString::Equals(str, noneStr))
			{
				*this = None();
			}
			else
			{
				*this = str;
			}
		}
		return true;
	}
}    // namespace Rift
