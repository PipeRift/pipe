// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/Checks.h"

#include <type_traits>


namespace Pipe::Core
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
		static Ret Dispatch(void* target, Args... args)
		{
			return (*(S*)target)(args...);
		}

		TFunction() : dispatcher(nullptr), target(nullptr) {}
		template<typename T>
		TFunction(T&& target) : dispatcher(&Dispatch<typename std::decay<T>::type>), target(&target)
		{}

		// Specialize for reference-to-function, to ensure that a valid pointer is stored
		TFunction(FunctionType function) : dispatcher(Dispatch<FunctionType>)
		{
			static_assert(sizeof(void*) == sizeof(function),
			    "It is not allowed to pass functions by reference. Use explicit function pointers: "
			    "i.e. foo(function) -> foo(&function)");
			target = reinterpret_cast<void*>(function);
		}

		Ret operator()(Args... args) const
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
}    // namespace Pipe::Core

namespace Pipe
{
	using namespace Pipe::Core;
}
