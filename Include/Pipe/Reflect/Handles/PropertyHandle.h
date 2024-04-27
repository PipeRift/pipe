// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Reflect/ArrayProperty.h"


namespace p
{
	struct PropertyHandle
	{
	protected:
		const Property& property;
		void* data = nullptr;


	public:
		PropertyHandle(const Property& property, void* data) : property{property}, data{data} {}
		PropertyHandle(const PropertyHandle& other) = default;

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

		virtual StringView GetDisplayName() const
		{
			return property.GetDisplayName();
		}

		Type* GetType() const
		{
			return property.GetType();
		}

		virtual void* GetPtr() const
		{
			return data;
		}

		bool IsValid() const
		{
			return data != nullptr;
		}

		operator bool() const
		{
			return IsValid();
		}
	};


	struct PropertyIndexHandle : public PropertyHandle
	{
		i32 index = NO_INDEX;

		PropertyIndexHandle(const PropertyHandle& other, i32 index)
		    : PropertyHandle(other), index(index)
		{
			Check(GetArrayProperty());
		}

		i32 GetIndex() const
		{
			return index;
		}

		StringView GetDisplayName() const override
		{
			static String indexDisplayName;
			indexDisplayName.clear();
			Strings::FormatTo(indexDisplayName, "{}", index);
			return indexDisplayName;
		}

		void* GetPtr() const override
		{
			auto& ArrayProp = *GetArrayProperty();
			return ArrayProp.GetItem(PropertyHandle::GetPtr(), index);
		}

		void* GetContainerPtr() const
		{
			return PropertyHandle::GetPtr();
		}
	};


	struct ValueHandle
	{
	protected:
		void* data               = nullptr;
		Type* type               = nullptr;
		const Property* property = nullptr;
		i32 index                = NO_INDEX;


	public:
		ValueHandle(void* data, Type* type) : data{data}, type{type} {}
		ValueHandle(void* data, const Property* property) : data{data}, property{property}
		{
			if (property)
			{
				type = property->GetType();
			}
		}
		ValueHandle(void* data, const Property* property, i32 index)
		    : data{data}, property{property}, index{index}
		{
			Check(GetArrayProperty());
			if (property)
			{
				type = property->GetType();
			}
		}
		ValueHandle(const ValueHandle& container, i32 index)
		    : ValueHandle(container.GetPtr(), container.GetProperty(), index)
		{}

		ValueHandle(const ValueHandle& other)            = default;
		ValueHandle& operator=(const ValueHandle& other) = default;

		const Property* GetProperty() const
		{
			return property;
		}

		bool IsArray() const
		{
			return !IsArrayItem() && property && property->HasFlag(Prop_Array);
		}
		bool IsArrayItem() const
		{
			return index != NO_INDEX;
		}

		const ArrayProperty* GetArrayProperty() const
		{
			if (property && property->HasFlag(Prop_Array))
			{
				return static_cast<const ArrayProperty*>(property);
			}
			return nullptr;
		}

		virtual StringView GetDisplayName() const
		{
			if (index != NO_INDEX)
			{
				static String indexDisplayName;
				indexDisplayName.clear();
				Strings::FormatTo(indexDisplayName, "{}", index);
				return indexDisplayName;
			}
			return property ? property->GetDisplayName() : StringView{};
		}

		Type* GetType() const
		{
			return type;
		}

		void* GetContainerPtr() const
		{
			Check(index != NO_INDEX);
			return data;
		}

		i32 GetIndex() const
		{
			return index;
		}

		void* GetPtr() const
		{
			const auto* arrayProp = GetArrayProperty();
			if (arrayProp && index != NO_INDEX)
			{
				return arrayProp->GetItem(data, index);
			}
			return data;
		}

		bool IsValid() const
		{
			return data != nullptr;
		}

		operator bool() const
		{
			return IsValid();
		}
	};
}    // namespace p
