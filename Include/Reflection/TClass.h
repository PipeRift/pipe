// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Class.h"
#include "Memory/OwnPtr.h"
#include "Object/ObjectBuilder.h"
#include "TProperty.h"
#include "TypeTraits.h"


namespace Rift::Refl
{
	// Class will be specialized for each type at compile time and store
	// the metadata for that type.
	template <typename T>
	class TClass : public Class
	{
		static_assert(IsObject<T>::value, "Type is not an Object!");

	private:
		static TClass _class;


	public:
		TClass() : Class()
		{
			T::__refl_Registry();
			T::__refl_RegistryProperties();
		}

	public:
		virtual OwnPtr<BaseObject, ObjectBuilder<BaseObject>> CreateInstance(
			const Ptr<BaseObject>& owner) override
		{
			if constexpr (std::is_same_v<T, BaseObject>)
			{
				return {};
			}
			auto instance = MakeOwned<T, ObjectBuilder<T>>(owner);
			return MoveTemp(instance);
		}

		static TClass* GetStatic()
		{
			return &_class;
		}
	};

	template <typename T>
	TClass<T> TClass<T>::_class{};
}	 // namespace Rift::Refl
