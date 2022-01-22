// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/TypeId.h"


namespace Rift::Refl
{
	/** Smallest reflection type */
	class CORE_API Type
	{
	protected:
		TypeId id;


	protected:
		Type() = default;

	public:
		Type(const Type&) = delete;
		Type& operator=(const Type&) = delete;

		TypeId GetId() const
		{
			return id;
		}
	};
}    // namespace Rift::Refl
