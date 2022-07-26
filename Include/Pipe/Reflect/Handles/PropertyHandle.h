// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/String.h"
#include "Pipe/Memory/OwnPtr.h"
#include "Pipe/Reflect/ArrayProperty.h"
#include "Pipe/Reflect/BaseClass.h"
#include "Pipe/Reflect/BaseStruct.h"
#include "Pipe/Reflect/Property.h"


namespace p
{
	struct PropertyHandle
	{
	protected:
		const Property& property;
		void* const container = nullptr;
		void* value           = nullptr;


	public:
		PropertyHandle(const Property& property, void* container)
		    : property{property}, container{container}
		{
			if (container)
			{
				value = property.GetDataPtr(container);
			}
		}

		PropertyHandle(const PropertyHandle& other)            = default;
		PropertyHandle& operator=(const PropertyHandle& other) = default;

		const Property& GetProperty() const
		{
			return property;
		}

		const ArrayProperty* GetArrayProperty() const
		{
			if (property.HasFlag(Prop_Array))
			{
				return static_cast<const ArrayProperty*>(&property);
			}
			return nullptr;
		}

		StringView GetDisplayName() const
		{
			return property.GetDisplayName();
		}

		Type* GetType() const
		{
			return property.GetType();
		}

		void* GetContainerPtr() const
		{
			return container;
		}

		void* GetPtr() const
		{
			return value;
		}

		bool IsValid() const
		{
			return value != nullptr;
		}

		operator bool() const
		{
			return IsValid();
		}
	};
}    // namespace p
