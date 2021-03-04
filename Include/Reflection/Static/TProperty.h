// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Events/Function.h"
#include "Reflection/ReflectionTags.h"
#include "Reflection/Static/Property.h"
#include "Reflection/Runtime/TPropertyHandle.h"
#include "Reflection/Static/Type.h"
#include "Strings/Name.h"

#include <functional>
#include <memory>


namespace Rift::Refl
{
	/**
	 * Static information about a property
	 */
	template <typename VariableT>
	class TProperty : public Property
	{
	public:
		using Access = TFunction<VariableT*(void*)>;

	private:
		Access access;


	public:
		TProperty(Type* type, Name typeName, Name name, Access&& access, ReflectionTags tags)
		    : Property(type, typeName, name, tags)
		    , access(access)
		{}

		virtual OwnPtr<PropertyHandle> CreateHandle(const Ptr<BaseObject>& instance) const override
		{
			const Type* type = GetInstanceType(instance);
			if (type == GetContainerType() || type->IsChildOf(GetContainerType()))
			{
				return OwnPtr<PropertyHandle>(
				    MakeOwned<TPropertyHandle<VariableT>>(instance, this, access));
			}
			return {};
		}

		// TODO: Improve struct property reflection
		/*virtual std::shared_ptr<PropertyHandle> CreateHandle(BaseStruct* instance) const override
		{
		    const Type* type = GetInstanceType(instance);
		    if (type == GetContainerType() || type->IsChildOf(GetContainerType()))
		    {
		        return std::shared_ptr<PropertyHandle>(
		            new TPropertyHandle<VariableT>(instance, this, access));
		    }
		    return {};
		}*/
	};
}    // namespace Rift::Refl
