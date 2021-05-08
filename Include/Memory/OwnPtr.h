// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Memory/PtrBuilder.h"
#include "Misc/Utility.h"
#include "TypeTraits.h"

#include <atomic>
#include <memory>


namespace Rift
{
	namespace Impl
	{
		// TODO: Use pool arena for counters
		using Deleter = void(void*);

		// Container that lives from when an owner is created to when the last weak has been reset
		struct PtrWeakCounter
		{
			Deleter* deleter;
			u32 weakCount = 0;

			PtrWeakCounter(Deleter& deleter) : deleter(&deleter) {}
			bool IsSet() const
			{
				return deleter != nullptr;
			}
		};

		struct Ptr;

		struct CORE_API OwnPtr
		{
			friend Ptr;

		protected:
			void* value             = nullptr;
			PtrWeakCounter* counter = nullptr;


		public:
			~OwnPtr()
			{
				Release();
			}

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

			void Release();


		protected:
			OwnPtr() = default;
			OwnPtr(void* value, Deleter& deleter) : value{value}
			{
				if (value)
				{
					counter = new PtrWeakCounter(deleter);
				}
			}


			void MoveFrom(OwnPtr&& other)
			{
				value         = other.value;
				counter       = other.counter;
				other.value   = nullptr;
				other.counter = nullptr;
			}
		};


		struct CORE_API Ptr
		{
		protected:
			void* value             = nullptr;
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
	}    // namespace Impl


	template <typename T>
	struct TPtr;


	/**
	 * Pointer Owner
	 * Contains an unique instance of T that is kept automatically removed on owner destruction.
	 */
	template <typename T>
	struct TOwnPtr : public Impl::OwnPtr
	{
		using Super = Impl::OwnPtr;

		template <typename T2>
		friend struct TOwnPtr;

#if BUILD_DEBUG
		// Pointer to value for debugging
		T* instance = nullptr;
#endif


	public:
		TOwnPtr() = default;
		explicit TOwnPtr(T* value, Impl::Deleter& deleter)
		    : Super(value, deleter)
#if BUILD_DEBUG
		    , instance(value)
#endif
		{}

		TOwnPtr(TOwnPtr&& other) noexcept
		{
#if BUILD_DEBUG
			instance       = reinterpret_cast<T*>(other.instance);
			other.instance = nullptr;
#endif
			MoveFrom(Move(other));
		}

		TOwnPtr& operator=(TOwnPtr&& other) noexcept
		{
#if BUILD_DEBUG
			instance       = other.instance;
			other.instance = nullptr;
#endif
			MoveFrom(Move(other));
			return *this;
		}

		/** Templates for down-casting */
		template <typename T2>
		TOwnPtr(TOwnPtr<T2>&& other) requires Derived<T2, T>
		{
#if BUILD_DEBUG
			instance       = reinterpret_cast<T*>(other.instance);
			other.instance = nullptr;
#endif
			MoveFrom(Move(other));
		}
		template <typename T2>
		TOwnPtr& operator=(TOwnPtr<T2>&& other) requires Derived<T2, T>
		{
#if BUILD_DEBUG
			instance       = other.instance;
			other.instance = nullptr;
#endif
			MoveFrom(Move(other));
			return *this;
		}

		T* Get() const
		{
			return IsValid() ? GetUnsafe() : nullptr;
		}

		T* GetUnsafe() const
		{
			return static_cast<T*>(value);
		}

		/** Cast a global pointer into another type. Will invalidate previous owner on success */
		template <typename T2>
		TOwnPtr<T2> Cast()
		{
			// If can be casted statically or dynamically
			if (IsValid() && (Convertible<T2, T> || dynamic_cast<T2*>(GetUnsafe()) != nullptr))
			{
				TOwnPtr<T2> newPtr{};
				newPtr.MoveFrom(Move(*this));
#if BUILD_DEBUG
				newPtr.instance = reinterpret_cast<T2*>(instance);
				instance        = nullptr;
#endif
				return newPtr;
			}
			return {};
		}

		template <typename T2 = T>
		TPtr<T2> AsPtr() const
		{
			if constexpr (Derived<T2, T>)
			{
				return {*this};
			}
			else if (IsValid() && dynamic_cast<T2*>(GetUnsafe()) != nullptr)
			{
				TPtr<T> ptr{*this};
				return ptr.template Cast<T2>();
			}
			return {};
		}

		T& operator*() const
		{
			return *GetUnsafe();
		}
		T* operator->() const
		{
			return GetUnsafe();
		}

		template <typename T2>
		bool operator==(T2* other) const
		{
			return value == other;
		}
		template <typename T2>
		bool operator==(const TOwnPtr<T2>& other) const
		{
			return value == other.GetUnsafe();
		}
		template <typename T2>
		bool operator==(const TPtr<T2>& other) const
		{
			return value == other.GetUnsafe();
		}
		template <typename T2>
		bool operator!=(T2* other) const
		{
			return value != other;
		}
		template <typename T2>
		bool operator!=(const TOwnPtr<T2>& other) const
		{
			return value != other.GetUnsafe();
		}
		template <typename T2>
		bool operator!=(const TPtr<T2>& other) const
		{
			return value != other.GetUnsafe();
		}

		template <typename... Args>
		static TOwnPtr<T> Make(Args&&... args) requires(HasCustomPtrBuilder<T>::value)
		{
			using Builder  = typename T::PtrBuilder<T>;
			auto* instance = Builder::New(std::forward<Args>(args)...);
			auto ptr       = TOwnPtr<T>(instance, Builder::Delete);
			Builder::PostNew(ptr);
			return Move(ptr);
		}

		template <typename... Args>
		static TOwnPtr<T> Make(Args&&... args) requires(!HasCustomPtrBuilder<T>::value)
		{
			using Builder  = TPtrBuilder<T>;
			auto* instance = Builder::New(std::forward<Args>(args)...);
			auto ptr       = TOwnPtr<T>(instance, Builder::Delete);
			Builder::PostNew(ptr);
			return Move(ptr);
		}
	};


	/**
	 * Weak pointers
	 * Instances will be removed if their ptr owner is released. In this case all Ptrs will be
	 * invalidated.
	 */
	template <typename T>
	struct TPtr : public Impl::Ptr
	{
		template <typename T2>
		friend struct TPtr;

		using Super = Impl::Ptr;

		TPtr() = default;
		TPtr(const TPtr& other) : Super(other) {}
		TPtr(TPtr&& other) noexcept : Super(Move(other)) {}

		TPtr& operator=(const TPtr& other)
		{
			CopyFrom(other);
			return *this;
		}

		TPtr& operator=(TPtr&& other) noexcept
		{
			MoveFrom(Move(other));
			return *this;
		}


		/** Templates for down-casting */

		template <typename T2>
		TPtr(const TOwnPtr<T2>& owner) requires Derived<T2, T> : Super(owner)
		{}

		template <typename T2>
		TPtr(const TPtr<T2>& other) requires Derived<T2, T> : Super(other)
		{}

		template <typename T2>
		TPtr(TPtr<T2>&& other) requires Derived<T2, T> : Super(Move(other))
		{}

		template <typename T2>
		TPtr& operator=(const TPtr<T2>& other) requires Derived<T2, T>
		{
			CopyFrom(other);
			return *this;
		}

		template <typename T2>
		TPtr& operator=(TPtr<T2>&& other) requires Derived<T2, T>
		{
			MoveFrom(Move(other));
			return *this;
		}

		T* Get() const
		{
			return IsValid() ? GetUnsafe() : nullptr;
		}

		T* GetUnsafe() const
		{
			return static_cast<T*>(value);
		}

		template <typename T2>
		TPtr<T2> Cast() const
		{
			if (IsValid() && (Convertible<T2, T> || dynamic_cast<T2*>(GetUnsafe()) == nullptr))
			{
				TPtr<T2> ptr{};
				ptr.CopyFrom(*this);
				return ptr;
			}
			return {};
		}


		T& operator*() const
		{
			return *GetUnsafe();
		}
		T* operator->() const
		{
			return GetUnsafe();
		}

		template <typename T2>
		bool operator==(T2* other) const
		{
			return value == other;
		}
		template <typename T2>
		bool operator==(const TOwnPtr<T2>& other) const
		{
			return value == other.GetUnsafe();
		}
		template <typename T2>
		bool operator==(const TPtr<T2>& other) const
		{
			return value == other.GetUnsafe();
		}
		template <typename T2>
		bool operator!=(T2* other) const
		{
			return value != other;
		}
		template <typename T2>
		bool operator!=(const TOwnPtr<T2>& other) const
		{
			return value != other.GetUnsafe();
		}
		template <typename T2>
		bool operator!=(const TPtr<T2>& other) const
		{
			return value != other.GetUnsafe();
		}
	};


	template <typename T, typename... Args>
	TOwnPtr<T> MakeOwned(Args&&... args) requires(!std::is_array_v<T>)
	{
		return TOwnPtr<T>::Make(std::forward<Args>(args)...);
	}

	// template <typename T>
	// TOwnPtr<T> MakeOwned(sizet size) requires(std::is_array_v<T>&& std::extent_v<T> == 0)
	//{
	//	using Elem = std::remove_extent_t<T>;
	//	return {Builder::NewArray(size)};
	//}
}    // namespace Rift
