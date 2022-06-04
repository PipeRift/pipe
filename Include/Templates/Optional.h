// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Core/Utility.h"


namespace Pipe
{
	/**
	 * When we have an optional value IsSet() returns true, and GetValue() is meaningful.
	 * Otherwise GetValue() is not meaningful.
	 */
	template<typename Type>
	struct TOptional
	{
	private:
		Type value;
		bool isSet;

	public:
		/** Construct an Type with no value; i.e. unset */
		TOptional() : isSet(false) {}

		/** Construct an Type with a valid value. */
		TOptional(const Type& value) : isSet{true}, value{value} {}
		TOptional(Type&& value) : isSet{true}, value{Move(value)} {}

		/** Copy/Move construction */
		TOptional(const TOptional& other) : isSet(other.isSet)
		{
			if (isSet)
			{
				value = other.value;
			}
		}
		TOptional(TOptional&& other) noexcept : isSet(Move(other.isSet))
		{
			if (isSet)
			{
				value = Move(other.value);
			}
		}

		TOptional& operator=(const TOptional& other)
		{
			isSet = other.isSet;
			if (isSet)
			{
				value = other.value;
			}
			return *this;
		}
		TOptional& operator=(TOptional&& other) noexcept
		{
			isSet = Move(other.isSet);
			if (isSet)
			{
				value = Move(other.value);
			}
			return *this;
		}

		TOptional& operator=(const Type& otherValue)
		{
			isSet = true;
			value = otherValue;
			return *this;
		}
		TOptional& operator=(Type&& otherValue)
		{
			isSet = true;
			value = Move(otherValue);
			return *this;
		}

		void Reset()
		{
			isSet = false;
		}

		template<typename... ArgsType>
		void Emplace(ArgsType&&... Args)
		{
			isSet = true;
			value = Type(Forward<ArgsType>(Args)...);
		}

		friend bool operator==(const TOptional& lhs, const TOptional& rhs)
		{
			return lhs.isSet == rhs.isSet && (!lhs->isSet || lhs.value == rhs.value);
		}
		friend bool operator!=(const TOptional& lhs, const TOptional& rhs)
		{
			return !(lhs == rhs);
		}

		/** @return true when the value is meaningful; false if calling GetValue() is undefined. */
		bool IsSet() const
		{
			return isSet;
		}
		explicit operator bool() const
		{
			return IsSet();
		}

		/** @return The optional value; undefined when IsSet() returns false. */
		const Type& Get() const
		{
			assert(IsSet() &&
			       "Called GetValue() on an unset Optional. Please either check IsSet() or "
			       "use Get(defaultValue) instead.");
			return value;
		}
		Type& Get()
		{
			assert(IsSet() &&
			       "Called Get() on an unset Optional. Please either check IsSet() or "
			       "use Get(defaultValue) instead.");
			return value;
		}

		const Type& operator*() const
		{
			return Get();
		}
		Type& operator*()
		{
			return Get();
		}
		const Type* operator->() const
		{
			return &Get();
		}
		Type* operator->()
		{
			return &Get();
		}

		/** @return The optional value when set; DefaultValue otherwise. */
		const Type& Get(const Type& defaultValue) const
		{
			return IsSet() ? value : defaultValue;
		}
	};
}    // namespace Pipe
