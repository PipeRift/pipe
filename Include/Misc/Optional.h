// Copyright 2015-2021 Piperift - All rights reserved

#pragma once


namespace Rift
{
	/**
	 * When we have an optional value IsSet() returns true, and GetValue() is meaningful.
	 * Otherwise GetValue() is not meaningful.
	 */
	template <typename Type>
	struct TOptional
	{
	private:
		Type value;
		bool bIsSet;

	public:
		/** Construct an Type with no value; i.e. unset */
		TOptional() : bIsSet(false) {}

		/** Construct an Type with a valid value. */
		TOptional(const Type& value) : bIsSet{true}, value{value} {}
		TOptional(Type&& value) : bIsSet{true}, value{Move(value)} {}

		/** Copy/Move construction */
		TOptional(const TOptional& other) : bIsSet(other.bIsSet)
		{
			if (bIsSet)
			{
				value = other.value;
			}
		}
		TOptional(TOptional&& other) noexcept : bIsSet(other.bIsSet)
		{
			if (bIsSet)
			{
				value = Move(other.value);
			}
		}

		TOptional& operator=(const TOptional& other)
		{
			if (&other != this)
			{
				bIsSet = other.bIsSet;
				if (bIsSet)
				{
					value = other.value;
				}
			}
			return *this;
		}
		TOptional& operator=(TOptional&& other) noexcept
		{
			if (&other != this)
			{
				bIsSet = other.bIsSet;
				if (bIsSet)
				{
					value = Move(other.value);
				}
			}
			return *this;
		}

		TOptional& operator=(const Type& otherValue)
		{
			if (&value != &otherValue)
			{
				bIsSet = true;
				value  = otherValue;
			}
			return *this;
		}
		TOptional& operator=(Type&& otherValue)
		{
			if (&value != &otherValue)
			{
				bIsSet = true;
				value  = Move(otherValue);
			}
			return *this;
		}

		void Reset()
		{
			bIsSet = false;
		}

		template <typename... ArgsType>
		void Emplace(ArgsType&&... Args)
		{
			bIsSet = true;
			value  = Type(Forward<ArgsType>(Args)...);
		}

		friend bool operator==(const TOptional& lhs, const TOptional& rhs)
		{
			return lhs.bIsSet == rhs.bIsSet && (!lhs->bIsSet || lhs.value == rhs.value);
		}
		friend bool operator!=(const TOptional& lhs, const TOptional& rhs)
		{
			return !(lhs == rhs);
		}

		/** @return true when the value is meaningful; false if calling GetValue() is undefined. */
		bool IsSet() const
		{
			return bIsSet;
		}
		explicit operator bool() const
		{
			return IsSet();
		}

		/** @return The optional value; undefined when IsSet() returns false. */
		const Type& GetValue() const
		{
			check(IsSet(),
			    TX("Called GetValue() on an unset TOptional. Please either check IsSet() or "
			       "use Get(DefaultValue) instead."));
			return value;
		}
		Type& GetValue()
		{
			check(IsSet(),
			    TX("Called GetValue() on an unset TOptional. Please either check IsSet() or "
			       "use Get(DefaultValue) instead."));
			return value;
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
