// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "BaseStruct.h"
#include "Serialization/ContextsFwd.h"
#include "TypeTraits.h"

namespace pipe
{
	template<typename Type>
	struct TPtr;
}

namespace pipe::refl
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

		void SerializeReflection(pipe::serl::CommonContext& ct) {}

	protected:
		virtual void BeforeDestroy() {}
	};
}    // namespace pipe::refl

namespace pipe
{
	using namespace pipe::refl;
}
