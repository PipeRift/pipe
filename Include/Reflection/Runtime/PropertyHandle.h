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
		BaseStruct* const structInstance = nullptr;
		TPtr<BaseClass> objInstance;
		const Property* const prop = nullptr;


	protected:
		PropertyHandle(const TPtr<BaseClass>& objInstance, const Property* prop)
		    : objInstance{objInstance}, prop{prop}
		{}
		PropertyHandle(BaseStruct* instance, const Property* prop)
		    : structInstance{instance}, prop{prop}
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

		void* GetInstance() const
		{
			if (UsesTOwnPtr())
			{
				return *objInstance;
			}
			return structInstance;
		}

		bool IsValid() const
		{
			return prop != nullptr && (UsesTOwnPtr() || structInstance);
		}

		bool UsesTOwnPtr() const
		{
			return objInstance.IsValid();
		}

		virtual ClassType* GetTypeDefinedWidgetClass()
		{
			return nullptr;
		}

		virtual void* GetRawValuePtr() const = 0;

		operator bool() const
		{
			return IsValid();
		}
	};
}    // namespace Rift::Refl
