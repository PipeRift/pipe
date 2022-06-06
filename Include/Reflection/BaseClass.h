// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "BaseStruct.h"
#include "Core/TypeTraits.h"
#include "Serialization/SerializationFwd.h"



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

		void StartDestroy()
		{
			BeforeDestroy();
		}

		ClassType* GetType() const;
		TPtr<Class> Self() const;

		void SerializeReflection(p::ReadWriter& ct) {}

	protected:
		virtual void BeforeDestroy() {}
	};
}    // namespace p
