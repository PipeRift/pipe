// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "BaseStruct.h"
#include "Serialization/ContextsFwd.h"
#include "TypeTraits.h"

namespace p
{
	template<typename Type>
	struct TPtr;
}

namespace p::refl
{
	class ClassType;
	class Class;


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

		refl::ClassType* GetType() const;
		TPtr<Class> Self() const;

		void SerializeReflection(p::serl::CommonContext& ct) {}

	protected:
		virtual void BeforeDestroy() {}
	};
}    // namespace p::refl

namespace p
{
	using namespace p::refl;
}
