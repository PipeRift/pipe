// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/String.h"
#include "PipeReflect.h"


namespace p
{
	struct ValueHandle
	{
	protected:
		void* container = nullptr;
		TypeId containerType;
		const TypeProperty* property = nullptr;
		i32 index                    = NO_INDEX;


	public:
		ValueHandle(void* container, TypeId containerType)
		    : container{container}, containerType{containerType}
		{}
		ValueHandle(void* container, TypeId containerType, const TypeProperty& property)
		    : container{container}, containerType{containerType}, property{&property}
		{}
		ValueHandle(void* data, const TypeProperty* property, i32 index)
		    : container{data}, property{property}, index{index}
		{
			// P_Check(GetArrayProperty());
		}
		ValueHandle(const ValueHandle& container, i32 index)
		    : ValueHandle(container.GetPtr(), container.GetProperty(), index)
		{}

		ValueHandle(const ValueHandle& other)            = default;
		ValueHandle& operator=(const ValueHandle& other) = default;

		const TypeProperty* GetProperty() const
		{
			return property;
		}

		bool IsArray() const
		{
			return !IsArrayItem() && property && property->HasFlag(PF_Array);
		}
		bool IsArrayItem() const
		{
			return index != NO_INDEX;
		}

		// const ArrayProperty* GetArrayProperty() const
		//{
		//	if (property && property->HasFlag(PF_Array))
		//	{
		//		return static_cast<const ArrayProperty*>(property);
		//	}
		//	return nullptr;
		// }

		virtual void GetDisplayName(String& name) const
		{
			if (index != NO_INDEX)
			{
				Strings::FormatTo(name, "{}", index);
			}
			else if (property)
			{
				name.append(Strings::ToSentenceCase(property->name.AsString()));
			}
		}

		TypeId GetType() const
		{
			return containerType;
		}

		void* GetContainer() const
		{
			P_Check(index != NO_INDEX);
			return container;
		}

		i32 GetIndex() const
		{
			return index;
		}

		void* GetPtr() const
		{
			// const auto* arrayProp = GetArrayProperty();
			// if (arrayProp && index != NO_INDEX)
			//{
			//	return arrayProp->GetItem(container, index);
			// }
			return container;
		}

		bool IsValid() const
		{
			return container != nullptr;
		}

		operator bool() const
		{
			return IsValid();
		}
	};
}    // namespace p
