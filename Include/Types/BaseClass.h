// Copyright 2015-2022 Piperift - All rights reserved
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
	class Class;

	template<typename Type>
	struct TPtr;


	class CORE_API BaseClass : public BaseStruct
	{
	protected:
		BaseClass() = default;

	public:
		virtual ~BaseClass() = default;

		void StartDestroy()
		{
			BeforeDestroy();
		}

		Refl::ClassType* GetClass() const;
		TPtr<Class> Self() const;

	protected:
		virtual void BeforeDestroy() {}
	};
}    // namespace Rift
