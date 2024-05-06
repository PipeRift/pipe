// Copyright 2015-2024 Piperift - All rights reserved

#include "PipeReflect.h"

#include "Pipe/Core/Platform.h"
#include "PipeECS.h"


namespace p
{
	static TArray<TypeId> ids;
	static TArray<TypeId> parentIds;
	static TArray<StringView> names;

	namespace details
	{
		i32 AssureSingleTypeId(TypeId id)
		{
			bool added      = false;
			const i32 index = ids.AddUniqueSorted(id, {}, &added);
			if (added)    // Not already registered?
			{
				parentIds.InsertUninitialized(index, 1);
				names.InsertUninitialized(index, 1);
				return index;
			}
			return NO_INDEX;
		}

		void SetTypeParentUnsafe(i32 index, TypeId parentId)
		{
			parentIds[index] = parentId;
		}

		void SetTypeNameUnsafe(i32 index, StringView name)
		{
			names[index] = name;
		}
	}    // namespace details


	i32 GetTypeIndex(TypeId id)
	{
		return ids.FindSorted(id);
	}

	TypeId GetParentId(TypeId id)
	{
		const i32 index = GetTypeIndex(id);
		return index ? parentIds[index] : TypeId{};
	}

	bool IsParentOf(TypeId parentId, TypeId childId)
	{
		TypeId currentId = childId;
		do
		{
			currentId = GetParentId(currentId);
			if (parentId == currentId)
				return true;
		} while (currentId.IsValid());
		return false;
	}

	StringView GetTypeName(TypeId id)
	{
		const i32 index = GetTypeIndex(id);
		return index ? names[index] : StringView{};
	}
}    // namespace p
