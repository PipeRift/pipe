// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "CoreTypes.h"
#include "Memory/OwnPtr.h"
#include "Reflection/Static/Property.h"
#include "Strings/String.h"
#include "Types/BaseClass.h"
#include "Types/BaseStruct.h"


namespace Rift::Refl
{
	struct PropertyHandle
	{
	protected:
		void* const container      = nullptr;
		const Property* const prop = nullptr;


	protected:
		PropertyHandle(Class* container, const Property* prop) : container{container}, prop{prop} {}
		PropertyHandle(Struct* container, const Property* prop) : container{container}, prop{prop}
		{}

	public:
		virtual ~PropertyHandle() = default;

		String GetName() const
		{
			if (prop)
				return prop->GetDisplayName();
			return "Invalid";
		}

		bool HasFlag(PropFlags flags) const
		{
			return prop ? prop->HasFlag(flags) : false;
		}

		void* GetContainer() const
		{
			return container;
		}

		bool IsValid() const
		{
			return prop && container;
		}

		void* GetValuePtr() const
		{
			return prop->GetValuePtr(container);
		}

		operator bool() const
		{
			return IsValid();
		}
	};
}    // namespace Rift::Refl
