// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Reflection/Runtime/PropertyHandle.h"


namespace Rift::Refl
{
	/**
	 * Base logic implementation for templated property handles.
	 * Use TPropertyHandle instead
	 */
	template<typename VariableT>
	struct TPropertyHandle : public PropertyHandle
	{
		using Access = std::function<VariableT*(void*)>;

		Access access;


	public:
		// TPropertyHandle() : PropertyHandle({}, nullptr) {}
		// Classs constructor
		TPropertyHandle(const TPtr<BaseClass>& instance, const Property* prop, const Access& access)
		    : PropertyHandle(instance, prop), access{access}
		{}

		// Structs constructor
		TPropertyHandle(BaseStruct* instance, const Property* prop, const Access& access)
		    : PropertyHandle(instance, prop), access{access}
		{}

		VariableT* GetValuePtr() const
		{
			if (IsValid())
			{
				return access(GetInstance());
			}
			return nullptr;
		}
		void* GetRawValuePtr() const override
		{
			return GetValuePtr();
		}

		bool GetValue(VariableT& value) const
		{
			VariableT* const valuePtr = GetValuePtr();
			if (valuePtr)
				value = *valuePtr;

			return valuePtr;
		}

		bool SetValue(const VariableT& value) const
		{
			VariableT* const valuePtr = GetValuePtr();
			if (valuePtr)
				*valuePtr = value;

			return valuePtr;
		}
	};
}    // namespace Rift::Refl
