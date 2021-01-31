// Copyright 2015-2021 Piperift - All rights reserved

#include "Strings/Name.h"

#include "Serialization/Archive.h"


namespace Rift
{
	const Hash<String> NameKey::hasher{};

	const String Name::noneStr{"none"};
	const Name::Id Name::noneId{0};

	size_t NameTable::Register(StringView str)
	{
		if (str.empty())
		{
			return Name::noneId;
		}

		// Calculate hash once
		NameKey key{str};
		ConstIterator FoundIt = table.find(key);
		if (FoundIt != table.end())
		{
			std::shared_lock lock{editTableMutex};
			return FoundIt->GetHash();
		}
		else
		{
			std::unique_lock lock{editTableMutex};
			return table.insert(MoveTemp(key)).first->GetHash();
		}
	}

	const String& NameTable::Find(size_t hash) const
	{
		// Ensure no other thread is editing the table
		std::shared_lock lock{ editTableMutex };
		ConstIterator foundIt = table.find({ hash });
		if (foundIt != table.end())
		{
			return foundIt->GetString();
		}
		// Should never reach
		return Name::NoneStr();
	}

	bool Name::Serialize(Archive& ar, const char* name)
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
}	 // namespace Rift
