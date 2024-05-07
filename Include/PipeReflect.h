// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Tag.h"
#include "Pipe/Reflect/TypeId.h"
#include "Pipe/Reflect/TypeName.h"


namespace p
{
	///////////////////////////////////////////////////////
	// Runtime reflection

	enum TypeFlags2
	{
		TF_None          = 0,         // -> No flags
		TF_NotSerialized = 1 << 0,    // -> This type ignores serialization
		TF_Abstract      = 1 << 1     // -> Reflection cant create instances on this type

		// Any other flags up to 64 bytes are available to the user
	};


	struct TypeProperty
	{
		using AccessFunc = void*(void*);

		TypeId typeId;
		u64 flags = 0;
		Tag name;
		AccessFunc* access = nullptr;
	};

	struct TypeOperations
	{
		using ReadFunc  = void(Reader&, void*);
		using WriteFunc = void(Writer&, void*);

		ReadFunc* read   = nullptr;
		WriteFunc* write = nullptr;
	};

	struct ArrayTypeOperations : public TypeOperations
	{
		using GetDataFunc    = void*(void*);
		using GetSizeFunc    = i32(void*);
		using GetItemFunc    = void*(void*, i32);
		using AddItemFunc    = void(void*, void*);
		using RemoveItemFunc = void(void*, i32);
		using ClearFunc      = void(void*);


		GetDataFunc* getData       = nullptr;
		GetSizeFunc* getSize       = nullptr;
		GetItemFunc* getItem       = nullptr;
		AddItemFunc* addItem       = nullptr;
		RemoveItemFunc* removeItem = nullptr;
		ClearFunc* clear           = nullptr;
	};


	PIPE_API TypeId GetParentId(TypeId id);
	PIPE_API TypeId GetParentId(TypeId id);
	PIPE_API bool IsParentOf(TypeId parentId, TypeId childId);
	PIPE_API StringView GetTypeName(TypeId id);


	namespace details
	{
		PIPE_API bool BeginTypeId(TypeId id);
		PIPE_API void EndTypeId();
		PIPE_API void SetTypeParent(TypeId parentId);
		PIPE_API void SetTypeName(StringView name);
		PIPE_API void SetTypeFlags(u64 flags);

		PIPE_API void AddTypeProperty(const TypeProperty& property);

		template<typename T, typename U>
		constexpr void AddTypeProperty(U T::*member, StringView name)
		{
			AddTypeProperty({.typeId = AssureTypeId<U>(),
			    .flags               = 0,
			    .name                = name,
			    .access              = [member](void* instance) {
                    (void*)&static_cast<T*>(instance)->*member;
			    }});
		}

		PIPE_API void SetTypeOperations(const TypeOperations* operations);
	}    // namespace details


	template<typename T>
	TypeId AssureTypeId();

	// Do not call directly.
	template<typename T>
	void RegisterTypeId(const TypeId typeId)
	{
		if (details::BeginTypeId(typeId))
		{
			if constexpr (HasSuper<T>::value)
			{
				details::SetTypeParent(AssureTypeId<typename T::Super>());
			}
			details::SetTypeName(GetTypeName<T>());
			details::EndTypeId();
		}
	}

	template<typename T>
	TypeId AssureTypeId()
	{
		// Static to only register once
		static TypeId typeId = []() {
			const TypeId typeId = GetTypeId<T>();
			RegisterTypeId<T>(typeId);
			return typeId;
		}();
		return typeId;
	}
};    // namespace p
