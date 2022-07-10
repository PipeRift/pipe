// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "BaseStruct.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Serialize/SerializationFwd.h"


namespace p
{
	class ClassType;
	class Class;
	template<typename Type>
	struct TPtr;


	class PIPE_API BaseClass : public BaseStruct
	{
	protected:
		BaseClass() = default;

	public:
		virtual ~BaseClass() = default;

		ClassType* GetType() const;
		TPtr<Class> Self() const;

		void SerializeReflection(p::ReadWriter& ct) {}
	};
}    // namespace p
