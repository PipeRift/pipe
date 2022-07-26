// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Core/Map.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/String.h"
#include "Pipe/Reflect/Builders/TypeInstance.h"
#include "Pipe/Reflect/Property.h"
#include "Pipe/Reflect/ReflectionFlags.h"
#include "Pipe/Reflect/ReflectionTraits.h"
#include "Pipe/Reflect/Type.h"


namespace p
{
	/** Smallest reflection type that contains all basic class or struct data */
	class DataType : public Type
	{
		template<typename T, typename Parent, TypeFlags flags, typename TType>
		friend struct TDataTypeBuilder;

	protected:
		TypeFlags flags = Type_NoFlag;

		DataType* parent = nullptr;
		TArray<DataType*> children;

		TArray<Property*> properties;


	protected:
		PIPE_API DataType(TypeCategory category) : Type(category | TypeCategory::Data) {}

	public:
		DataType(const DataType&)            = delete;
		DataType& operator=(const DataType&) = delete;
		PIPE_API ~DataType() override;

		/** Type */

		PIPE_API bool IsChildOf(const DataType* other) const;

		template<typename T>
		bool IsChildOf() const
		{
			static_assert(
			    IsStruct<T>() || IsClass<T>(), "IsChildOf only valid with Structs or Classes.");
			return IsChildOf(static_cast<DataType*>(TTypeInstance<T>::GetType()));
		}

		PIPE_API bool IsParentOf(const DataType* other) const
		{
			return other && other->IsChildOf(this);
		}

		PIPE_API bool HasFlag(TypeFlags flag) const;
		PIPE_API bool HasAllFlags(TypeFlags inFlags) const;
		PIPE_API bool HasAnyFlags(TypeFlags inFlags) const;

		/** Properties */
		PIPE_API const Property* FindProperty(const Name& propertyName) const;
		PIPE_API const TArray<Property*>& GetSelfProperties() const;
		PIPE_API void GetProperties(TArray<Property*>& outProperties) const;

		PIPE_API DataType* GetParent() const
		{
			return parent;
		}
		PIPE_API const TArray<DataType*>& GetChildren() const;
		PIPE_API void GetChildrenDeep(TArray<DataType*>& outChildren) const;
		PIPE_API DataType* FindChild(const Name& className) const;
	};
}    // namespace p
