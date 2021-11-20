// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "CoreTypes.h"
#include "Reflection/ReflectionFlags.h"
#include "Reflection/ReflectionTraits.h"
#include "Reflection/Registry/TypeInstance.h"
#include "Reflection/Static/Property.h"
#include "Reflection/Static/Type.h"
#include "Strings/String.h"


namespace Rift::Refl
{
	using PropertyMap = TMap<Name, Property*>;


	/** Smallest reflection type that contains all basic class or struct data */
	class DataType : public Type
	{
		template<typename T, typename Parent, TypeFlags flags, typename TType>
		friend struct TDataTypeBuilder;

	protected:
		Name name;
		TypeFlags flags = Type_NoFlag;

		DataType* parent = nullptr;
		TArray<DataType*> children;

		PropertyMap properties{};


	protected:
		DataType() = default;

	public:
		DataType(const DataType&) = delete;
		DataType& operator=(const DataType&) = delete;
		virtual ~DataType()
		{
			for (auto& it : properties)
			{
				delete it.second;
			}
		}

		/** Type */
		CORE_API const Name& GetName() const;
		CORE_API const String& GetSName() const;

		CORE_API bool IsChildOf(const DataType* other) const;

		template<typename T>
		bool IsChildOf() const
		{
			static_assert(
			    IsStruct<T>() || IsClass<T>(), "IsChildOf only valid with Structs or Classes.");
			return IsChildOf(static_cast<DataType*>(InternalGetType<T>()));
		}

		CORE_API bool IsParentOf(const DataType* other) const
		{
			return other && other->IsChildOf(this);
		}

		CORE_API bool HasFlag(PropFlags flag) const;
		CORE_API bool HasAllFlags(PropFlags inFlags) const;
		CORE_API bool HasAnyFlags(PropFlags inFlags) const;

		/** Properties */
		CORE_API const Property* FindProperty(const Name& propertyName) const;
		CORE_API void GetOwnProperties(PropertyMap& outProperties) const;
		CORE_API void GetAllProperties(PropertyMap& outProperties) const;


	protected:
		void __GetAllChildren(TArray<DataType*>& outChildren);
		DataType* __FindChild(const Name& className) const;
	};
}    // namespace Rift::Refl
