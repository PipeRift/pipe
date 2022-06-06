// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Reflection/Property.h"


namespace p
{
	/**
	 * Static information about an array property
	 */
	class ArrayProperty : public Property
	{
		template<typename T, typename Parent, TypeFlags flags, typename TType>
		friend struct TDataTypeBuilder;

	public:
		using Property::Property;

		using GetDataFunc    = void*(void*);
		using GetSizeFunc    = i32(void*);
		using GetItemFunc    = void*(void*, i32);
		using AddItemFunc    = void(void*, void*);
		using RemoveItemFunc = void(void*, i32);
		using EmptyFunc      = void(void*);


	protected:
		GetDataFunc* getData       = nullptr;
		GetSizeFunc* getSize       = nullptr;
		GetItemFunc* getItem       = nullptr;
		AddItemFunc* addItem       = nullptr;
		RemoveItemFunc* removeItem = nullptr;
		EmptyFunc* empty           = nullptr;

	public:

		CORE_API void* GetData(void* container) const;
		CORE_API i32 GetSize(void* container) const;
		CORE_API void* GetItem(void* container, i32 index) const;
		CORE_API void AddItem(void* container, void* item) const;
		CORE_API void RemoveItem(void* container, i32 index) const;
		CORE_API void Empty(void* container) const;
	};

	inline void* ArrayProperty::GetData(void* container) const
	{
		return getData(container);
	}
	inline i32 ArrayProperty::GetSize(void* container) const
	{
		return getSize(container);
	}
	inline void* ArrayProperty::GetItem(void* container, i32 index) const
	{
		return getItem(container, index);
	}
	inline void ArrayProperty::AddItem(void* container, void* item) const
	{
		addItem(container, item);
	}
	inline void ArrayProperty::RemoveItem(void* container, i32 index) const
	{
		removeItem(container, index);
	}
	inline void ArrayProperty::Empty(void* container) const
	{
		empty(container);
	}
}    // namespace p
