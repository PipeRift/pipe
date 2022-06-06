// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Reflection/Handles/PropertyHandle.h"


namespace p
{
	template<typename U>
	struct TPropertyHandle : public PropertyHandle
	{
		using T = Mut<U>;

	public:
		TPropertyHandle(const Property& prop, void* container) : PropertyHandle(prop, container)
		{
			// TODO: Check value types and their inheritance
		}

		T* Get()
		{
			return static_cast<T*>(value);
		}

		const T* Get() const
		{
			return static_cast<T*>(value);
		}

		void Set(const T& value)
		{
			if (T* ptr = Get())
			{
				*ptr = value;
			}
		}

		void Set(T&& value)
		{
			if (T* ptr = Get())
			{
				*ptr = Move(value);
			}
		}

		T& operator*()
		{
			return *Get();
		}
		const T& operator*() const
		{
			return *Get();
		}
		T& operator->()
		{
			return *Get();
		}
		const T& operator->() const
		{
			return *Get();
		}
	};
}    // namespace p
