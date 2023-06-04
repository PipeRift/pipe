// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "EventHandle.h"
#include "Function.h"
#include "Pipe/Core/Log.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Reflect/Class.h"


namespace p::core
{
	template<typename... Params>
	class TBroadcast
	{
	protected:
		using Method   = void(Params...);
		using Function = TFunction<void(Params...)>;

		using MethodPtr = void (*)(Params...);
		template<typename Type>
		using MemberMethodPtr = void (Type::*)(Params...);


		struct BaseListener
		{
			u64 id;
			Function method;
		};
		struct RawListener : public BaseListener
		{
			void* instance;
		};
		struct PtrListener : public BaseListener
		{
			TPtr<Class> instance;
		};

		mutable TArray<RawListener> rawListeners{};
		mutable TArray<PtrListener> ptrListeners{};


	public:
		TBroadcast()                           = default;
		explicit TBroadcast(const TBroadcast&) = default;
		TBroadcast(TBroadcast&&)               = default;

		/** Broadcast to all binded functions */
		void Broadcast(const Params&... params)
		{
			for (RawListener& listener : rawListeners)
			{
				listener.method(params...);
			}

			for (auto& listener : ptrListeners)
			{
				if (listener.instance)
				{
					listener.method(params...);
				}
			}
		}

		// #TODO: Scoped binding

		/** Binds an static function. Must be unbinded manually. */
		EventHandle Bind(Function method) const
		{
			if (method)
			{
				EventHandle handle{};
				rawListeners.Add({handle.Id(), Move(method), nullptr});
				return handle;
			}

			Warning("Couldn't bind delegate");
			return EventHandle::Invalid();
		}

		/** Binds a member function. Must be unbinded manually. */
		template<typename Type>
		EventHandle Bind(Type* instance, MemberMethodPtr<Type> method) const
		{
			if (instance && method)
			{
				if constexpr (IsClass<Type>())
				{
					return Bind<Type>(instance->Self(), Move(method));
				}
				else
				{
					return Bind([instance, method](Params... params) {
						(instance->*method)(params...);
					});
				}
			}

			Warning("Couldn't bind delegate");
			return EventHandle::Invalid();
		}

		/** Binds an object's function. Gets unbinded when the objects is destroyed
		 */
		template<typename Type>
		EventHandle Bind(TPtr<Type> instance, MemberMethodPtr<Type> method) const
		{
			if (instance && method)
			{
				Type* const instancePtr = *instance;
				Function func           = [instancePtr, method](Params... params) {
                    (instancePtr->*method)(params...);
				};

				EventHandle handle{};
				ptrListeners.Add({handle.Id(), Move(func), instance});
				return handle;
			}

			Warning("Couldn't bind delegate");
			return EventHandle::Invalid();
		}

		bool Unbind(const EventHandle& handle) const
		{
			if (!handle)
			{
				return false;
			}

			return rawListeners.RemoveIfSwap([handle](const auto& listener) {
				return listener.id == handle.Id();
			}) > 0;
		}

		template<typename Type>
		bool Unbind(TPtr<Type> instance) const
		{
			if (instance) [[likely]]
			{
				return ptrListeners.RemoveIfSwap([instance](const auto& listener) {
					return !listener.instance || listener.instance == instance;
				}) > 0;
			}
			return false;
		}

		template<typename Type>
		bool Unbind(Type* instance) const
		{
			if (instance) [[likely]]
			{
				return rawListeners.RemoveIfSwap([instance](const auto& listener) {
					return listener.instance == instance;
				}) > 0;
			}
			return false;
		}

		bool UnbindAll() const
		{
			if (!rawListeners.IsEmpty() || !ptrListeners.IsEmpty()) [[likely]]
			{
				rawListeners.Clear();
				ptrListeners.Clear();
				return true;
			}
			return false;
		}

		void operator()(const Params&... params)
		{
			Broadcast(Forward<Params>(params)...);
		}
	};
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
