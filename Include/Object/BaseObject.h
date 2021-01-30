// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "BaseStruct.h"
#include "TypeTraits.h"



namespace Rift
{
	namespace Refl
	{
		class Class;
	}
	class Object;

	template <typename Type>
	struct Ptr;

	class CORE_API BaseObject : public BaseStruct
	{
	protected:
		BaseObject() : BaseStruct() {}

	public:
		void StartDestroy()
		{
			BeforeDestroy();
		}

		Refl::Class* GetType() const;
		Ptr<Object> Self() const;

	protected:
		virtual void BeforeDestroy() {}
	};


	template <typename Type>
	using IsObject = std::is_base_of<BaseObject, Type>;
}	 // namespace Rift
