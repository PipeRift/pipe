// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "BaseStruct.h"
#include "Serialization/ContextsFwd.h"
#include "TypeTraits.h"

namespace Pipe
{
	template<typename Type>
	struct TPtr;
}

namespace Pipe::Refl
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

		Refl::ClassType* GetType() const;
		TPtr<Class> Self() const;

		void SerializeReflection(Pipe::Serl::CommonContext& ct) {}

	protected:
		virtual void BeforeDestroy() {}
	};
}    // namespace Pipe::Refl

namespace Pipe
{
	using namespace Pipe::Refl;
}
