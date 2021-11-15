#pragma once

#include "PCH.h"

#include <memory>


namespace Rift
{
	template<typename T, typename D = std::default_delete<T>>
	struct TUniquePtr
	{
		using Pointer = typename std::unique_ptr<T>::pointer;

	private:
		std::unique_ptr<T> ptr;


	public:
		constexpr TUniquePtr() noexcept = default;
		explicit constexpr TUniquePtr(Pointer p) noexcept : ptr(p) {}
		constexpr TUniquePtr(nullptr_t) noexcept : ptr(nullptr) {}
		TUniquePtr& operator=(nullptr_t) noexcept
		{
			Reset();
			return *this;
		}

		TUniquePtr(const TUniquePtr&) = delete;
		TUniquePtr& operator=(const TUniquePtr&) = delete;

		template<class D2 = D>
		TUniquePtr& operator=(TUniquePtr&& other) noexcept requires(IsMoveAssignable<D2>)
		{
			if (Get() != other.Get())
			{
				ptr = Move(other.ptr);
			}
			return *this;
		}

		template<class T2, class D2>
		TUniquePtr& operator=(TUniquePtr<T2, D2>&& other) noexcept requires(
		    !std::is_array_v<
		        T2> && std::is_assignable_v<D&, D2> && Convertible<typename TUniquePtr<T2, D2>::Pointer, Pointer>)
		{
			ptr = Move(other.ptr);
			return *this;
		}

		void Swap(TUniquePtr& other) noexcept
		{
			ptr.swap(other.ptr);
		}

		void Reset()
		{
			ptr.reset();
		}

		Pointer Get() const noexcept
		{
			return ptr.get();
		}

		Pointer& operator*() const noexcept
		{
			return *Get();
		}

		Pointer operator->() const noexcept
		{
			return Get();
		}

		explicit operator bool() const noexcept
		{
			return static_cast<bool>(Get());
		}
	};


	template<typename T, typename... Args>
	TUniquePtr<T> MakeUnique(Args&&... args) requires(!std::is_array_v<T>)
	{
		return TUniquePtr<T>{new T(std::forward<Args>(args)...)};
	}

	template<typename T>
	TUniquePtr<T> MakeUnique(sizet size) requires(std::is_array_v<T>&& std::extent_v<T> == 0)
	{
		return TUniquePtr<T>{new T[size]()};
	}

	template<typename T, typename... Args>
	TUniquePtr<T> MakeUnique(Args&&...) requires(std::extent_v<T> != 0) = delete;
}    // namespace Rift
