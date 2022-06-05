// Copyright 2015-2022 Piperift - All rights reserved

#include "Memory/OwnPtr.h"


namespace p
{
	void BaseOwnPtr::Delete()
	{
		if (!counter)
		{
			return;
		}

		counter->deleter(value);
		if (counter->weakCount <= 0)
		{
			delete counter;
		}
		else
		{
			// With this weaks know the value has been deleted
			counter->deleter = nullptr;
		}
		counter = nullptr;
		value   = nullptr;
	}

	void Ptr::Reset()
	{
		if (counter)
		{
			ResetNoCheck(counter->IsSet());
		}
	}

	bool Ptr::IsValid() const
	{
		if (counter)
		{
			if (counter->IsSet())
			{
				return true;
			}
			const_cast<Ptr*>(this)->ResetNoCheck(false);
		}
		return false;
	}

	Ptr::Ptr(const BaseOwnPtr& owner)
	{
		value   = owner.value;
		counter = owner.counter;
		if (counter)
		{
			++counter->weakCount;
		}
	}
	Ptr::Ptr(const Ptr& other)
	{
		value   = other.value;
		counter = other.counter;
		if (counter)
		{
			++counter->weakCount;
		}
	}
	Ptr::Ptr(Ptr&& other) noexcept
	{
		value         = other.value;
		counter       = other.counter;
		other.counter = nullptr;
	}

	void Ptr::MoveFrom(Ptr&& other)
	{
		if (counter != other.counter)
		{
			Reset();
			value         = other.value;
			counter       = other.counter;
			other.counter = nullptr;
		}
		else    // If equals, we reset previous anyway
		{
			other.Reset();
		}
	}

	void Ptr::CopyFrom(const Ptr& other)
	{
		if (counter != other.counter)
		{
			Reset();
			value   = other.value;
			counter = other.counter;
			if (counter)
			{
				++counter->weakCount;
			}
		}
	}

	void Ptr::ResetNoCheck(const bool isSet)
	{
		// counter check is reducing one and then checking
		if (!isSet && counter->weakCount <= 1)
		{
			delete counter;
		}
		else
		{
			--counter->weakCount;
		}
		counter = nullptr;
		value   = nullptr;
	}
}    // namespace p
