// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Memory/PtrBuilder.h"
#include "TypeTraits.h"

#include <atomic>
#include <memory>


namespace Rift
{
	namespace Impl
	{
		// Container that lives from when an owner is created to when the last weak has been reset
		struct PtrWeakCounter
		{
			// Owner and weak references
			std::atomic<bool> bIsSet = true;
			std::atomic<u32> weaks = 0;
		};

		struct Ptr;

		struct CORE_API OwnPtr
		{
			friend Ptr;

		protected:
			void* value = nullptr;
			PtrWeakCounter* counter = nullptr;


		public:
			bool IsValid() const
			{
				return counter != nullptr;
			}

			operator bool() const
			{
				return IsValid();
			};

			const PtrWeakCounter* GetCounter() const
			{
				return counter;
			}

		protected:
			OwnPtr() = default;
			OwnPtr(void* value) : value{value}
			{
				if (value)
				{
					counter = new PtrWeakCounter();
				}
			}

			void MoveFrom(OwnPtr&& other)
			{
				value = other.value;
				counter = other.counter;
				other.value = nullptr;
				other.counter = nullptr;
			}
		};


		struct CORE_API Ptr
		{
		protected:
			void* value = nullptr;
			PtrWeakCounter* counter = nullptr;


		public:
			~Ptr()
			{
				Reset();
			}

			void Reset();
			bool IsValid() const;

			operator bool() const
			{
				return IsValid();
			};

			const PtrWeakCounter* GetCounter() const
			{
				return counter;
			}

			Ptr& operator=(TYPE_OF_NULLPTR)
			{
				Reset();
				return *this;
			};

		protected:
			Ptr() = default;
			Ptr(const OwnPtr& owner);
			Ptr(const Ptr& other);
			Ptr(Ptr&& other) noexcept;

			void MoveFrom(Ptr&& other);
			void CopyFrom(const Ptr& other);

		private:
			void __ResetNoCheck(const bool bIsSet);
		};
	}	 // namespace Impl


	template <typename T>
	struct Ptr;


	/**
	 * Pointer Owner
	 * Contains an unique instance of T that is kept automatically removed on owner destruction.
	 */
	template <typename T, template <typename BT> typename Builder = PtrBuilder>
	struct OwnPtr : public Impl::OwnPtr
	{
		using Super = Impl::OwnPtr;
		using TBuilder = Builder<T>;

		template <typename T2, template <typename BT2> typename Builder2>
		friend struct OwnPtr;

	public:
		OwnPtr() = default;
		explicit OwnPtr(T* value) : Super(value) {}

		OwnPtr(OwnPtr&& other) noexcept
		{
			MoveFrom(MoveTemp(other));
		}

		OwnPtr& operator=(OwnPtr&& other) noexcept
		{
			MoveFrom(MoveTemp(other));
			return *this;
		}

		/** Templates for down-casting */
		template <typename T2>
		OwnPtr(OwnPtr<T2, Builder>&& other) requires Derived<T2, T>
		{
			// static_assert(
			//	std::is_same_v<T2, T> || std::is_base_of_v<T2, T>, "Type is not down-castable!");
			MoveFrom(MoveTemp(other));
		}
		template <typename T2>
		OwnPtr& operator=(OwnPtr<T2, Builder>&& other) requires Derived<T2, T>
		{
			// static_assert(std::is_same_v<T2, T> || std::is_convertible_v<T2, T>,
			//	"Type is not down-castable!");
			MoveFrom(MoveTemp(other));
			return *this;
		}

		~OwnPtr()
		{
			Release();
		}

		T* Get() const
		{
			return IsValid() ? operator*() : nullptr;
		}

		/** Cast a global pointer into another type. Will invalidate previous owner on success */
		template <typename T2>
		OwnPtr<T2, Builder> Cast()
		{
			// If can be casted statically or dynamically
			if (IsValid() && (std::is_convertible_v<T2, T> || dynamic_cast<T2*>(**this) != nullptr))
			{
				OwnPtr<T2, Builder> newPtr{};
				newPtr.MoveFrom(MoveTemp(*this));
				return newPtr;
			}
			return {};
		}

		template <typename T2 = T>
		Ptr<T2> AsPtr() const
		{
			if constexpr (std::is_same_v<T2, T> || std::is_convertible_v<T2, T>)
			{
				return {*this};
			}
			else if (IsValid() && dynamic_cast<T2*>(**this) != nullptr)
			{
				Ptr<T> ptr{*this};
				return ptr.template Cast<T2>();
			}
			return {};
		}

		void Release()
		{
			if (counter)
			{
				TBuilder::Delete(value);
				value = nullptr;

				counter->bIsSet = false;
				if (counter->weaks <= 0)
				{
					delete counter;
				}
				counter = nullptr;
			}
		}

		T* operator*() const
		{
			return static_cast<T*>(value);
		}
		T* operator->() const
		{
			return static_cast<T*>(value);
		}

		template <typename T2>
		bool operator==(T2* other) const
		{
			return **this == other;
		}
		template <typename T2>
		bool operator==(const OwnPtr<T2, Builder>& other) const
		{
			return operator==(*other);
		}
		template <typename T2>
		bool operator==(const Ptr<T2>& other) const
		{
			return operator==(*other);
		}
		template <typename T2>
		bool operator!=(T2* other) const
		{
			return **this != other;
		}
		template <typename T2>
		bool operator!=(const OwnPtr<T2, Builder>& other) const
		{
			return operator!=(*other);
		}
		template <typename T2>
		bool operator!=(const Ptr<T2>& other) const
		{
			return operator!=(*other);
		}
	};


	/**
	 * Weak pointers
	 * Instances will be removed if their ptr owner is released. In this case all Ptrs will be
	 * invalidated.
	 */
	template <typename T>
	struct Ptr : public Impl::Ptr
	{
		template <typename T2>
		friend struct Ptr;

		using Super = Impl::Ptr;

		Ptr() = default;
		Ptr(const Ptr& other) : Super(other) {}
		Ptr(Ptr&& other) noexcept : Super(MoveTemp(other)) {}

		Ptr& operator=(const Ptr& other)
		{
			CopyFrom(other);
			return *this;
		}

		Ptr& operator=(Ptr&& other) noexcept
		{
			MoveFrom(MoveTemp(other));
			return *this;
		}


		/** Templates for down-casting */

		template <typename T2, template <typename BT> typename Builder>
		Ptr(const OwnPtr<T2, Builder>& owner) : Super(owner)
		{
			static_assert(std::is_same_v<T2, T> || std::is_convertible_v<T2, T>,
				"Type is not down-castable!");
		}

		template <typename T2>
		Ptr(const Ptr<T2>& other) : Super(other)
		{
			static_assert(std::is_same_v<T2, T> || std::is_convertible_v<T2, T>,
				"Type is not down-castable!");
		}

		template <typename T2>
		Ptr(Ptr<T2>&& other) : Super(MoveTemp(other))
		{
			static_assert(std::is_same_v<T2, T> || std::is_convertible_v<T2, T>,
				"Type is not down-castable!");
		}

		template <typename T2>
		Ptr& operator=(const Ptr<T2>& other)
		{
			static_assert(std::is_same_v<T2, T> || std::is_convertible_v<T2, T>,
				"Type is not down-castable!");
			CopyFrom(other);
			return *this;
		}

		template <typename T2>
		Ptr& operator=(Ptr<T2>&& other)
		{
			static_assert(std::is_same_v<T2, T> || std::is_convertible_v<T2, T>,
				"Type is not down-castable!");
			MoveFrom(MoveTemp(other));
			return *this;
		}


		T* operator*() const
		{
			return static_cast<T*>(value);
		}
		T* operator->() const
		{
			return static_cast<T*>(value);
		}

		T* Get() const
		{
			return IsValid() ? operator*() : nullptr;
		}

		template <typename T2>
		Ptr<T2> Cast() const
		{
			if (IsValid() && (std::is_convertible_v<T2, T> || dynamic_cast<T2*>(**this) == nullptr))
			{
				Ptr<T2> ptr{};
				ptr.CopyFrom(*this);
				return ptr;
			}
			return {};
		}

		template <typename T2>
		bool operator==(T2* other) const
		{
			return **this == other;
		}
		template <typename T2, template <typename BT> typename Builder>
		bool operator==(const OwnPtr<T2, Builder>& other) const
		{
			return operator==(*other);
		}
		template <typename T2>
		bool operator==(const Ptr<T2>& other) const
		{
			return operator==(*other);
		}
		template <typename T2>
		bool operator!=(T2* other) const
		{
			return **this != other;
		}
		template <typename T2, template <typename BT> typename Builder>
		bool operator!=(const OwnPtr<T2, Builder>& other) const
		{
			return operator!=(*other);
		}
		template <typename T2>
		bool operator!=(const Ptr<T2>& other) const
		{
			return operator!=(*other);
		}
	};


	template <typename T, template <typename BT> typename Builder = PtrBuilder, typename... Args,
		EnableIfT<!std::is_array_v<T>, i32> = 0>
	OwnPtr<T, Builder> MakeOwned(Args&&... args)
	{
		return OwnPtr<T, Builder>(Builder<T>::New(std::forward<Args>(args)...));
	}

	template <typename T, template <typename BT> typename Builder = PtrBuilder,
		EnableIfT<std::is_array_v<T> && std::extent_v<T> == 0, i32> = 0>
	OwnPtr<T, Builder> MakeOwned(size_t size)
	{
		using Elem = std::remove_extent_t<T>;
		return {Builder<T>::NewArray(size)};
	}

	template <typename T, typename TBuilder = PtrBuilder<T>, typename... Args,
		EnableIfT<std::extent_v<T> != 0, i32> = 0>
	void MakeOwned(Args&&...) = delete;
}	 // namespace Rift
