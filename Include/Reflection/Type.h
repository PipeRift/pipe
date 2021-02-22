// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "CoreTypes.h"
#include "Reflection/Property.h"
#include "Reflection/ReflectionTags.h"
#include "Strings/String.h"


namespace Rift::Refl
{
	using PropertyMap = TMap<Name, Property*>;

	/** Smallest reflection type that contains all basic class or struct data */
	class CORE_API Type
	{
		template <typename T, typename Parent, typename TType, ReflectionTags tags>
		friend struct TTypeBuilder;

	protected:
		Name name;
		ReflectionTags tags = ReflectionTags::None;

		Type* parent = nullptr;
		TArray<Type*> children;

		PropertyMap properties{};


	public:
		Type()            = default;
		Type(const Type&) = delete;
		Type& operator=(const Type&) = delete;
		virtual ~Type()
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

		bool IsChildOf(const Type* other) const;

		template <typename T>
		bool IsChildOf() const
		{
			return IsChildOf(T::StaticType());
		}

		bool IsParentOf(const Type* other) const
		{
			return other && other->IsChildOf(this);
		}


		/** Properties */
		const Property* FindProperty(const Name& propertyName) const;
		void GetOwnProperties(PropertyMap& outProperties) const;
		void GetAllProperties(PropertyMap& outProperties) const;


	protected:
		void __GetAllChildren(TArray<Type*>& outChildren);
		Type* __FindChild(const Name& className) const;
	};
}    // namespace Rift::Refl
