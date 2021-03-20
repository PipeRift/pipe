// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "CoreTypes.h"
#include "Reflection/ReflectionTags.h"
#include "Reflection/ReflectionTraits.h"
#include "Reflection/Registry/TypeInstance.h"
#include "Reflection/Static/Property.h"
#include "Reflection/Static/Type.h"
#include "Strings/String.h"


namespace Rift::Refl
{
	using PropertyMap = TMap<Name, Property*>;

	/** Smallest reflection type that contains all basic class or struct data */
	class CORE_API DataType : public Type
	{
		template <typename T, typename Parent, typename TType, ReflectionTags tags>
		friend struct TDataTypeBuilder;

	protected:
		Name name;
		ReflectionTags tags = ReflectionTags::None;

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
		const Name& GetName() const
		{
			return name;
		}
		const String& GetSName() const
		{
			return GetName().ToString();
		}

		bool HasTag(ReflectionTags tag) const
		{
			return (tags & tag) > 0;
		}

		bool IsChildOf(const DataType* other) const;

		template <typename T>
		bool IsChildOf() const
		{
			static_assert(
			    IsStruct<T>() || IsClass<T>(), "IsChildOf only valid with Structs or Classes.");
			return IsChildOf(static_cast<DataType*>(InternalGetType<T>()));
		}

		bool IsParentOf(const DataType* other) const
		{
			return other && other->IsChildOf(this);
		}


		/** Properties */
		const Property* FindProperty(const Name& propertyName) const;
		void GetOwnProperties(PropertyMap& outProperties) const;
		void GetAllProperties(PropertyMap& outProperties) const;


	protected:
		void __GetAllChildren(TArray<DataType*>& outChildren);
		DataType* __FindChild(const Name& className) const;
	};
}    // namespace Rift::Refl
