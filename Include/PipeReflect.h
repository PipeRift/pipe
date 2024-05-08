// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Tag.h"
#include "Pipe/Reflect/TypeId.h"
#include "Pipe/Reflect/TypeName.h"


namespace p
{
	// clang-format off
	template<typename T>
	concept MemberBuildType = requires() {
		{ T::BuildType() };
	};
	// clang-format on

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
		u64 flags          = 0;
		AccessFunc* access = nullptr;
		Tag name;
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

	namespace details
	{
		PIPE_API i32 GetTypeIndex(TypeId id);
	}

	PIPE_API TView<TypeId> GetRegisteredTypeIds(TypeId id);
	PIPE_API bool IsTypeRegistered(TypeId id);
	PIPE_API TypeId GetParentType(TypeId id);
	PIPE_API bool IsParentTypeOf(TypeId parentId, TypeId childId);
	PIPE_API StringView GetTypeName(TypeId id);
	PIPE_API u64 GetTypeFlags(TypeId id);
	PIPE_API TView<TypeProperty> GetTypeProperties(TypeId id);


#pragma region TypeRegistration
	/** To register types, define either of the following functions for that type:
	 * - A member static function BuildType() inside of the type to register.
	 * - An external function BuildType(const T*) where T is the type.
	 *   The parameter is used as a dummy for overloading. Will always be null.
	 *
	 * NOTE: BuildType() is optional! Only needed to add properties, operations, etc.
	 *       Types can also be of any kind, including templates.
	 *
	 * Examples:
	 * - A member BuildType():
	 *     struct A {
	 *         bool value = false;
	 *         static void BuildType()
	 *         {
	 *             SetTypeName("A");
	 *             AddTypeProperty("value", &A::value);
	 *         }
	 *     };
	 *
	 * - An external BuildType():
	 *     struct B {
	 *         bool value = false;
	 *     };
	 *     // NOTE: External should be placed -outside- of any namespaces (except "p" which is
	 * optional) void BuildType(const B*)
	 *     {
	 *         SetTypeName("B");
	 *         AddTypeProperty("value", &B::value);
	 *     }
	 */

	// Resolve the right BuildType function to call
	template<typename T>
	void BuildType(const T*) requires(MemberBuildType<T>)
	{
		T::BuildType();
	}
	template<typename T>
	void BuildType(const T*) requires(!MemberBuildType<T>)
	{}


	template<typename T>
	TypeId RegisterTypeId();    // Forward declaration

	PIPE_API bool BeginTypeId(TypeId id);
	PIPE_API void EndTypeId();
	PIPE_API void SetTypeParent(TypeId parentId);
	PIPE_API void SetTypeName(StringView name);
	PIPE_API void SetTypeFlags(u64 flags);
	PIPE_API void AddTypeProperty(const TypeProperty& property);
	template<typename T, typename U>
	constexpr void AddTypeProperty(U T::*member, StringView name, u64 flags = 0)
	{
		AddTypeProperty({.typeId = RegisterTypeId<U>(),
		    .flags               = flags,
		    .name                = name,
		    .access              = [member](void* instance) {
                (void*)&static_cast<T*>(instance)->*member;
		    }});
	}
	PIPE_API void SetTypeOperations(const TypeOperations* operations);


	template<typename T>
	TypeId RegisterTypeId()
	{    // Static to only register once
		static bool bRegistered = false;
		const TypeId typeId     = GetTypeId<T>();
		if (!bRegistered && BeginTypeId(typeId))
		{
			bRegistered = true;

			// Obvious defaults
			if constexpr (HasSuper<T>::value)
			{
				SetTypeParent(RegisterTypeId<typename T::Super>());
			}
			SetTypeName(GetTypeName<T>());

			BuildType<T>(nullptr);
			EndTypeId();
		}
		return typeId;
	}
#pragma endregion TypeRegistration
};    // namespace p
