// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/StringView.h"
#include "Pipe/Reflect/TypeId.h"
#include "Pipe/Reflect/TypeName.h"


namespace p
{
	///////////////////////////////////////////////////////
	// Runtime reflection

	namespace details
	{
		PIPE_API i32 AssureSingleTypeId(TypeId id);
		PIPE_API void SetTypeParentUnsafe(i32 index, TypeId parentId);
		PIPE_API void SetTypeNameUnsafe(i32 index, StringView name);
	}    // namespace details


	template<typename T>
	TypeId AssureTypeId();

	PIPE_API TypeId GetParentId(TypeId id);
	PIPE_API TypeId GetParentId(TypeId id);
	PIPE_API bool IsParentOf(TypeId parentId, TypeId childId);
	PIPE_API StringView GetTypeName(TypeId id);


	///////////////////////////////////////////////////////
	// Implementations

	template<typename T>
	TypeId AssureTypeId()
	{
		// Static to only register once
		static TypeId typeId = []() {
			TypeId parentId{p::Undefined{}};
			if constexpr (HasSuper<T>::value)
			{
				parentId = AssureTypeId<typename T::Super>();
			}
			else
			{
				parentId = TypeId{};
			}

			const TypeId typeId = GetTypeId<T>();
			const i32 index     = details::AssureSingleTypeId(typeId);
			if (index != NO_INDEX)    // Invalid or already registered
			{
				details::SetTypeParentUnsafe(index, parentId);
				details::SetTypeNameUnsafe(index, GetTypeName<T>());
			}
			return typeId;
		}();
		return typeId;
	}
};    // namespace p
