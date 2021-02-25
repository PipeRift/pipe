// Copyright 2015-2021 Piperift - All rights reserved

#pragma once


namespace Rift
{
	/**
	 * When we have an optional value IsSet() returns true, and GetValue() is meaningful.
	 * Otherwise GetValue() is not meaningful.
	 */
	template <typename Type>
	struct Optional
	{
	private:
		Type value;
		bool isSet;

	public:
		/** Construct an Type with no value; i.e. unset */
		Optional() : isSet(false) {}

		/** Construct an Type with a valid value. */
		Optional(const Type& value) : isSet{true}, value{value} {}
		Optional(Type&& value) : isSet{true}, value{Move(value)} {}

		/** Copy/Move construction */
		Optional(const Optional& other) : isSet(other.isSet)
		{
			if (isSet)
			{
				value = other.value;
			}
		}
		Optional(Optional&& other) noexcept : isSet(Move(other.isSet))
		{
			if (isSet)
			{
				value = Move(other.value);
			}
		}

		Optional& operator=(const Optional& other)
		{
			isSet = other.isSet;
			if (isSet)
			{
				value = other.value;
			}
			return *this;
		}
		Optional& operator=(Optional&& other) noexcept
		{
			isSet = Move(other.isSet);
			if (isSet)
			{
				value = Move(other.value);
			}
			return *this;
		}

		Optional& operator=(const Type& otherValue)
		{
			isSet = true;
			value = otherValue;
			return *this;
		}
		Optional& operator=(Type&& otherValue)
		{
			isSet = true;
			value = Move(otherValue);
			return *this;
		}

		void Reset()
		{
			isSet = false;
		}

		template <typename... ArgsType>
		void Emplace(ArgsType&&... Args)
		{
			isSet = true;
			value = Type(Forward<ArgsType>(Args)...);
		}

		friend bool operator==(const Optional& lhs, const Optional& rhs)
		{
			return lhs.isSet == rhs.isSet && (!lhs->isSet || lhs.value == rhs.value);
		}
		friend bool operator!=(const Optional& lhs, const Optional& rhs)
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
		const Type& GetValue() const
		{
			assert(IsSet() &&
			       "Called GetValue() on an unset Optional. Please either check IsSet() or "
			       "use Get(defaultValue) instead.");
			return value;
		}
		Type& GetValue()
		{
			assert(IsSet() &&
			       "Called GetValue() on an unset Optional. Please either check IsSet() or "
			       "use Get(defaultValue) instead.");
			return value;
		}

		const Type& operator*() const
		{
			return GetValue();
		}
		Type& operator*()
		{
			return GetValue();
		}
		const Type* operator->() const
		{
			return &GetValue();
		}
		Type* operator->()
		{
			return &GetValue();
		}

		/** @return The optional value when set; DefaultValue otherwise. */
		const Type& Get(const Type& defaultValue) const
		{
			return IsSet() ? value : defaultValue;
		}
	};
}    // namespace Rift
