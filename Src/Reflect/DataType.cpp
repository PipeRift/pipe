// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Reflect/DataType.h"

#include "Pipe/Reflect/Property.h"


namespace p
{
	DataType::~DataType()
	{
		for (Property* prop : properties)
		{
			prop->~Property();
		}
	}

	bool DataType::IsChildOf(const DataType* other) const
	{
		if (!other || !parent)
			return false;

		const DataType* current = parent;
		while (current)
		{
			if (other == current)
				return true;

			current = current->parent;
		}
		return false;
	}

	DataType* DataType::GetParent() const
	{
		return parent;
	}

	bool DataType::IsParentOf(const DataType* other) const
	{
		return other && other->IsChildOf(this);
	}

	const Property* DataType::FindProperty(const Tag& propertyName) const
	{
		const auto* prop = properties.Find([propertyName](Property* prop) {
			return prop->GetName() == propertyName;
		});
		return prop ? *prop : nullptr;
	}

	const TArray<Property*>& DataType::GetSelfProperties() const
	{
		return properties;
	}

	void DataType::GetProperties(TArray<Property*>& outProperties) const
	{
		if (parent)
		{
			parent->GetProperties(outProperties);
		}
		outProperties.Append(GetSelfProperties());
	}

	bool DataType::IsEmpty() const
	{
		return properties.Size() <= 0 && (!parent || parent->IsEmpty());
	}

	bool DataType::HasFlag(TypeFlags flag) const
	{
		return HasAnyFlags(flag);
	}

	bool DataType::HasAllFlags(TypeFlags inFlags) const
	{
		return (flags & inFlags) == inFlags;
	}

	bool DataType::HasAnyFlags(TypeFlags inFlags) const
	{
		return (flags & inFlags) > 0;
	}

	const TArray<DataType*>& DataType::GetChildren() const
	{
		return children;
	}
	void DataType::GetChildrenDeep(TArray<DataType*>& outChildren) const
	{
		outChildren.Reserve(outChildren.Size() + children.Size());
		for (auto* const child : children)
		{
			outChildren.Add(child);
			child->GetChildrenDeep(outChildren);
		}
	}

	DataType* DataType::FindChild(const Tag& className) const
	{
		if (className.IsNone())
			return nullptr;

		for (auto* const child : children)
		{
			if (child->GetName() == className.AsString())    // TODO: Use Tags
			{
				return child;
			}
			else if (DataType* found = child->FindChild(className))
			{
				return found;
			}
		}
		return nullptr;
	}
}    // namespace p
