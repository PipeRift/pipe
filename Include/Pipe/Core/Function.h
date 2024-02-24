// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Checks.h"

#include <type_traits>


namespace p
{
	template<typename>
	struct TFunction;    // intentionally not defined

	template<typename Ret, typename... Args>
	struct TFunction<Ret(Args...)>
	{
		using Dispatcher   = Ret(void*, Args...);
		using FunctionType = Ret(Args...);

		// A pointer to the static function that will call the wrapped invocable object
		Dispatcher* dispatcher;
		void* target;

		// Dispatch() is instantiated by the TransientFunction constructor,
		// which will store a pointer to the function in dispatcher.
		template<typename S>
		static P_FORCEINLINE Ret Dispatch(void* target, Args... args)
		{
			return (*(S*)target)(args...);
		}

		constexpr TFunction() : dispatcher(nullptr), target(nullptr) {}
		template<typename T>
		constexpr TFunction(T&& target)
		    : dispatcher(&Dispatch<typename std::decay<T>::type>), target(&target)
		{}

		// Specialize for reference-to-function, to ensure that a valid pointer is stored
		constexpr TFunction(FunctionType function) : dispatcher(Dispatch<FunctionType>)
		{
			static_assert(sizeof(void*) == sizeof(function),
			    "It is not allowed to pass functions by reference. Use explicit function pointers: "
			    "i.e. foo(function) -> foo(&function)");
			target = reinterpret_cast<void*>(function);
		}

		constexpr Ret operator()(Args... args) const
		{
			Check(IsBound() && "Can't call an unbound TFunction.");
			return dispatcher(target, args...);
		}

		bool IsBound() const
		{
			return dispatcher != nullptr;
		}

		operator bool() const
		{
			return IsBound();
		}

		bool operator==(const TFunction<Ret(Args...)>& other) const
		{
			return dispatcher == other.dispatcher && target == other.target;
		}

		bool operator!=(const TFunction<Ret(Args...)>& other) const
		{
			return dispatcher != other.dispatcher || target != other.target;
		}
	};
}    // namespace p
