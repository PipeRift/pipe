// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "CoreTypes.h"
#include "Memory/OwnPtr.h"
#include "Object/BaseObject.h"
#include "Object/BaseStruct.h"
#include "Reflection/Static/Property.h"
#include "Strings/String.h"


namespace Rift::Refl
{
	struct PropertyHandle
	{
	protected:
		BaseStruct* const structInstance = nullptr;
		TPtr<BaseObject> objInstance;
		const Property* const prop = nullptr;


	protected:
		PropertyHandle(const TPtr<BaseObject>& objInstance, const Property* prop)
		    : objInstance{objInstance}
		    , prop{prop}
		{}
		PropertyHandle(BaseStruct* instance, const Property* prop)
		    : structInstance{instance}
		    , prop{prop}
		{}

	public:
		virtual ~PropertyHandle() = default;

		String GetName() const
		{
			if (prop)
				return prop->GetDisplayName();
			return "Invalid";
		}

		bool HasTag(ReflectionTags tag) const
		{
			return prop ? prop->HasTag(tag) : false;
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
