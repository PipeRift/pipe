// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "BaseStruct.h"
#include "Pipe/Memory/OwnPtr.h"
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


	// For shared export purposes, we separate pointers from the exported Class
	struct PIPE_API ClassOwnership
	{
		TPtr<BaseClass> self;
		TPtr<BaseClass> owner;
		static inline TPtr<BaseClass> nextOwner;


		ClassOwnership() : self{}, owner{Move(nextOwner)} {}

		const TPtr<BaseClass>& GetSelf() const
		{
			return self;
		}
		const TPtr<BaseClass>& GetOwner() const
		{
			return owner;
		}
	};
}    // namespace p
