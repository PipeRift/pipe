#pragma once

#include "PCH.h"

#include <memory>


template<typename T, class D /* = default_delete<_Ty> */>
struct TUniquePtr
{
	using Pointer = std::unique_ptr<T>::pointer;

private:
	std::unique_ptr<T> ptr;

public:
	constexpr TUniquePtr() noexcept : ptr{} = default;
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
	    !std::is_array<
	        T2> && std::is_assignable<D&, D2> && IsConvertible<typename TUniquePtr<T2, D2>::Pointer, Pointer>)
	{
		ptr = Move(other.ptr);
		return *this;
	}

	void swap(TUniquePtr& other) noexcept
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
