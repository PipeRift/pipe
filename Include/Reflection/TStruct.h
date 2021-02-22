// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Reflection/Struct.h"
#include "TProperty.h"

#include <type_traits>


namespace Rift::Refl
{
	/** TStruct will be specialized for each type at compile time and store
	 * the metadata for that type
	 */
	template <typename T>
	class TStruct : public Struct
	{
		static_assert(std::is_convertible<T, Rift::Struct>::value, "Type does not inherit Struct!");

	private:
		static TStruct* _instance;


	public:
		TStruct() : Struct() {}

		BaseStruct* New() const override
		{
			return new T();
		}

		static TStruct* GetStatic()
		{
			return _instance;
		}
	};

	template <typename T>
	TStruct<T>* TStruct<T>::_instance = T::InitType();
}    // namespace Rift::Refl
