// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/TypeId.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Memory/PtrBuilder.h"
#include "PipeMemory.h"

#include <atomic>


namespace p
{
	namespace Internal
	{
		// TODO: Use pool arena for counters
		using Deleter = void(Arena& arena, void* ptr);

		// Container that lives from when an owner is created to when the last weak has been reset
		struct PtrWeakCounter
		{
			Deleter* deleter;
			u32 weakCount = 0;
			Arena& arena;

			PtrWeakCounter(Arena& arena, Deleter* deleter) : deleter(deleter), arena{arena} {}
			bool IsSet() const
			{
				return deleter != nullptr;
			}

			void Delete()
			{
				this->~PtrWeakCounter();
				p::Free<Internal::PtrWeakCounter>(arena, this, 1);
			}
		};
	}    // namespace Internal


	struct PIPE_API BaseOwnPtr
	{
		friend struct Ptr;

	protected:
		void* value                       = nullptr;
		Internal::PtrWeakCounter* counter = nullptr;


	public:
		BaseOwnPtr(BaseOwnPtr&& other) noexcept
		{
			MoveFromUnsafe(Move(other));
		}
		BaseOwnPtr& operator=(BaseOwnPtr&& other) noexcept
		{
			Delete();
			MoveFromUnsafe(Move(other));
			return *this;
		}
		~BaseOwnPtr()
		{
			Delete();
		}

		bool IsValid() const
		{
			return counter != nullptr;
		}

		operator bool() const
		{
			return IsValid();
		};

		const Internal::PtrWeakCounter* GetCounter() const
		{
			return counter;
		}

		void Delete();

		void* GetRaw()
		{
			return value;
		}
		const void* GetRaw() const
		{
			return value;
		}

		void MoveFromUnsafe(BaseOwnPtr&& other)
		{
			value         = other.value;
			counter       = other.counter;
			other.value   = nullptr;
			other.counter = nullptr;
		}

	protected:
		BaseOwnPtr() = default;
		// Initialization from parent
		BaseOwnPtr(Arena& arena, void* value, Internal::Deleter* deleter) : value{value}
		{
			if (value)
			{
				counter = new (p::Alloc<Internal::PtrWeakCounter>(arena))
				    Internal::PtrWeakCounter(arena, deleter);
			}
		}
	};


	struct PIPE_API Ptr
	{
	protected:
		void* value                       = nullptr;
		Internal::PtrWeakCounter* counter = nullptr;


	public:
		Ptr() = default;
		Ptr(const Ptr& other);
		~Ptr()
		{
			Reset();
		}

	protected:
		Ptr(const BaseOwnPtr& owner);
		Ptr(Ptr&& other) noexcept;


	public:
		void Reset();
		bool IsValid() const;

		operator bool() const
		{
			return IsValid();
		};

		const Internal::PtrWeakCounter* GetCounter() const
		{
			return counter;
		}

		Ptr& operator=(TYPE_OF_NULLPTR)
		{
			Reset();
			return *this;
		};

		void* GetRaw()
		{
			return value;
		}
		const void* GetRaw() const
		{
			return value;
		}

		void MoveFromUnsafe(Ptr&& other);
		void CopyFromUnsafe(const Ptr& other);

	private:
		void ResetNoCheck(const bool bIsSet);
	};


	///////////////////////////////////////////////////
	// Templated Pointer types

	struct OwnPtr;
	template<typename T>
	struct TPtr;


	/**
	 * Pointer Owner
	 * Contains an unique instance of T that is kept automatically removed on owner destruction.
	 */
	template<typename T>
	struct TOwnPtr : public BaseOwnPtr
	{
		using Super = BaseOwnPtr;

		template<typename T2>
		friend struct TOwnPtr;

#if P_DEBUG
		// Pointer to value for debugging
		T* instance = nullptr;
#endif


	public:
		TOwnPtr() = default;
		explicit TOwnPtr(Arena& arena, T* value, Internal::Deleter* deleter)
		    : Super(arena, value, deleter)
#if P_DEBUG
		    , instance(value)
#endif
		{}

		TOwnPtr(TOwnPtr&& other) noexcept
		{
			MoveFromUnsafe(Move(other));
		}
		TOwnPtr(OwnPtr&& other) noexcept;

		TOwnPtr& operator=(TOwnPtr&& other) noexcept
		{
			Delete();
			MoveFromUnsafe(Move(other));
			return *this;
		}

		TOwnPtr& operator=(OwnPtr&& other) noexcept;

		/** Templates for down-casting */
		template<typename T2>
		TOwnPtr(TOwnPtr<T2>&& other) noexcept requires Derived<T2, T>
		{
			MoveFromUnsafe(Move(other));
		}
		template<typename T2>
		TOwnPtr& operator=(TOwnPtr<T2>&& other) noexcept requires Derived<T2, T>
		{
			Delete();
			MoveFromUnsafe(Move(other));
			return *this;
		}

		T* Get() const
		{
			return static_cast<T*>(value);
		}

		/** Casts this OwnPtr into another. On success, it will transfer its ownership, else, the
		 * ownership is kept and the new ptr is invalid.
		 * */
		template<typename T2>
		TOwnPtr<T2> Transfer()
		{
			if (Cast<T2*>(Get()))
			{
				TOwnPtr<T2> newPtr{};
				newPtr.MoveFromUnsafe(Move(*this));
				return newPtr;
			}
			return {};
		}

		TPtr<T> AsPtr() const
		{
			return TPtr<T>{*this};
		}

		T& operator*() const
		{
			return *Get();
		}
		T* operator->() const
		{
			return Get();
		}

		template<typename T2>
		bool operator==(T2* other) const
		{
			return value == other;
		}
		template<typename T2>
		bool operator==(const TOwnPtr<T2>& other) const
		{
			return value == other.Get();
		}
		template<typename T2>
		bool operator==(const TPtr<T2>& other) const
		{
			return value == other.Get();
		}
		template<typename T2>
		bool operator!=(T2* other) const
		{
			return value != other;
		}
		template<typename T2>
		bool operator!=(const TOwnPtr<T2>& other) const
		{
			return value != other.Get();
		}
		template<typename T2>
		bool operator!=(const TPtr<T2>& other) const
		{
			return value != other.Get();
		}

		template<typename... Args>
		static TOwnPtr<T> Make(Args&&... args) requires(HasCustomPtrBuilder<T>::value)
		{
			Arena& arena   = GetCurrentArena();
			using Builder  = typename T::template PtrBuilder<T>;
			auto* instance = Builder::New(arena, p::Forward<Args>(args)...);
			auto ptr       = TOwnPtr<T>(arena, instance, Builder::Delete);
			return Move(ptr);
		}

		template<typename... Args>
		static TOwnPtr<T> Make(Args&&... args) requires(!HasCustomPtrBuilder<T>::value)
		{
			Arena& arena   = GetCurrentArena();
			using Builder  = TPtrBuilder<T>;
			auto* instance = Builder::New(arena, p::Forward<Args>(args)...);
			auto ptr       = TOwnPtr<T>(arena, instance, &Builder::Delete);
			return Move(ptr);
		}

		template<typename T2>
		void MoveFromUnsafe(TOwnPtr<T2>&& other)
		{
			Super::MoveFromUnsafe(Move(other));
#if P_DEBUG
			instance       = static_cast<T*>(value);
			other.instance = nullptr;
#endif
		}
	};


	/**
	 * Weak pointers
	 * Instances will be removed if their ptr owner is released. In this case all Ptrs will be
	 * invalidated.
	 */
	template<typename T>
	struct TPtr : public Ptr
	{
		template<typename T2>
		friend struct TPtr;

		using Super = Ptr;

		TPtr() = default;
		TPtr(const TPtr& other) : Super(other) {}
		TPtr(TPtr&& other) noexcept : Super(Move(other)) {}

		TPtr& operator=(const TPtr& other)
		{
			CopyFromUnsafe(other);
			return *this;
		}

		TPtr& operator=(TPtr&& other) noexcept
		{
			MoveFromUnsafe(Move(other));
			return *this;
		}


		/** Templates for down-casting */

		template<typename T2>
		TPtr(const TOwnPtr<T2>& owner) requires Derived<T2, T>
		    : Super(owner)
		{}

		template<typename T2>
		TPtr(const TPtr<T2>& other) requires Derived<T2, T>
		    : Super(other)
		{}

		template<typename T2>
		TPtr(TPtr<T2>&& other) noexcept requires Derived<T2, T>
		    : Super(Move(other))
		{}

		template<typename T2>
		TPtr& operator=(const TPtr<T2>& other) requires Derived<T2, T>
		{
			CopyFromUnsafe(other);
			return *this;
		}

		template<typename T2>
		TPtr& operator=(TPtr<T2>&& other) noexcept requires Derived<T2, T>
		{
			MoveFromUnsafe(Move(other));
			return *this;
		}

		T* Get() const
		{
			return static_cast<T*>(value);
		}

		T& operator*() const
		{
			return *Get();
		}
		T* operator->() const
		{
			return Get();
		}

		template<typename T2>
		bool operator==(T2* other) const
		{
			return value == other;
		}
		template<typename T2>
		bool operator==(const TOwnPtr<T2>& other) const
		{
			return value == other.Get();
		}
		template<typename T2>
		bool operator==(const TPtr<T2>& other) const
		{
			return value == other.Get();
		}
		template<typename T2>
		bool operator!=(T2* other) const
		{
			return value != other;
		}
		template<typename T2>
		bool operator!=(const TOwnPtr<T2>& other) const
		{
			return value != other.Get();
		}
		template<typename T2>
		bool operator!=(const TPtr<T2>& other) const
		{
			return value != other.Get();
		}
	};


	///////////////////////////////////////////////////
	// Non-templated Pointer types

	struct PIPE_API OwnPtr : public BaseOwnPtr
	{
		using Super = BaseOwnPtr;

	private:
		TypeId typeId;


	public:
		constexpr OwnPtr() = default;
		template<typename T>
		OwnPtr(TOwnPtr<T>&& other) noexcept
		{
			MoveFromUnsafe(Move(other));
			typeId = GetTypeId<T>();
		}
		OwnPtr(OwnPtr&& other) noexcept
		{
			MoveFromUnsafe(Move(other));
			typeId       = Move(other.typeId);
			other.typeId = TypeId::None();
		}

		template<typename T>
		OwnPtr& operator=(TOwnPtr<T>&& other) noexcept
		{
			Delete();
			MoveFromUnsafe(Move(other));
			typeId = GetTypeId<T>();
			return *this;
		}

		OwnPtr& operator=(OwnPtr&& other) noexcept
		{
			Delete();
			MoveFromUnsafe(Move(other));
			typeId       = Move(other.typeId);
			other.typeId = TypeId::None();
			return *this;
		}

		template<typename T>
		T* Get()
		{
			return IsType<T>() ? static_cast<T*>(value) : nullptr;
		}
		template<typename T>
		const T* Get() const
		{
			return IsType<T>() ? static_cast<const T*>(value) : nullptr;
		}
		template<typename T>
		T* GetUnsafe()
		{
			return static_cast<T*>(value);
		}
		template<typename T>
		const T* GetUnsafe() const
		{
			return static_cast<const T*>(value);
		}
		void* Get()
		{
			return value;
		}
		const void* Get() const
		{
			return value;
		}

		template<typename T>
		TPtr<T> AsPtr() const
		{
			return TPtr<T>{*this};
		}


		TypeId GetId() const
		{
			return typeId;
		}

		template<typename T>
		bool IsType() const
		{
			return typeId == GetTypeId<T>();
		}
	};


	template<typename T>
	inline TOwnPtr<T>::TOwnPtr(OwnPtr&& other) noexcept
	{
		P_CheckMsg(other.IsType<T>(), "Type doesn't match!");
		Super::MoveFromUnsafe(Move(other));
#if P_DEBUG
		instance = static_cast<T*>(value);
#endif
	}

	template<typename T>
	inline TOwnPtr<T>& TOwnPtr<T>::operator=(OwnPtr&& other) noexcept
	{
		P_CheckMsg(other.IsType<T>(), "Type doesn't match!");
		Delete();
		Super::MoveFromUnsafe(Move(other));
#if P_DEBUG
		instance = static_cast<T*>(value);
#endif
	}


	template<typename T, typename... Args>
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


	inline sizet GetHash(const OwnPtr& ptr) noexcept
	{
		return GetHash(ptr.GetRaw());
	}
	inline sizet GetHash(const Ptr& ptr) noexcept
	{
		return GetHash(ptr.GetRaw());
	}
}    // namespace p
