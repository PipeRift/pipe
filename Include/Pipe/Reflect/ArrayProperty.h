// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Reflect/Property.h"


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
		using ClearFunc      = void(void*);


	protected:
		GetDataFunc* getData       = nullptr;
		GetSizeFunc* getSize       = nullptr;
		GetItemFunc* getItem       = nullptr;
		AddItemFunc* addItem       = nullptr;
		RemoveItemFunc* removeItem = nullptr;
		ClearFunc* clear           = nullptr;

	public:

		PIPE_API void* GetData(void* container) const;
		PIPE_API i32 GetSize(void* container) const;
		PIPE_API void* GetItem(void* container, i32 index) const;
		PIPE_API void AddItem(void* container, void* item) const;
		PIPE_API void RemoveItem(void* container, i32 index) const;
		PIPE_API void Clear(void* container) const;
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
	inline void ArrayProperty::Clear(void* container) const
	{
		clear(container);
	}
}    // namespace p
