// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "CoreTypes.h"
#include "Memory/OwnPtr.h"
#include "Reflection/Property.h"
#include "Strings/String.h"
#include "Types/BaseClass.h"
#include "Types/BaseStruct.h"


namespace Rift::Refl
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

		PropertyHandle(const PropertyHandle& other) = default;
		PropertyHandle& operator=(const PropertyHandle& other) = default;

		const Property& GetProperty() const
		{
			return property;
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
}    // namespace Rift::Refl
