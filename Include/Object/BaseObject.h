// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "BaseStruct.h"
#include "TypeTraits.h"


namespace Rift
{
	namespace Refl
	{
		class ClassType;
	}
	class Object;

	template <typename Type>
	struct TPtr;


	class CORE_API BaseObject : public BaseStruct
	{
	protected:
		BaseObject() = default;

	public:
		virtual ~BaseObject() = default;

		void StartDestroy()
		{
			BeforeDestroy();
		}

		Refl::ClassType* GetClass() const;
		TPtr<Object> Self() const;

	protected:
		virtual void BeforeDestroy() {}
	};
}    // namespace Rift
