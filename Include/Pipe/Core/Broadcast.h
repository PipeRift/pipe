// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Function.h"
#include "Pipe/Core/Log.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Memory/OwnPtr.h"

// TODO: Remove this dependency
#include "PipeReflect.h"


namespace p
{
	struct PIPE_API EventHandle
	{
	private:
		static u64 counter;

		static u64 NewId()
		{
			if (counter == 0)
				++counter;
			return counter++;
		}

		u64 id;

		/** Used for invalidation */
		EventHandle(u64 customId) : id(customId) {}

	public:
		static EventHandle Invalid()
		{
			return {0};
		}

		EventHandle() : id(NewId()) {}

		u64 Id() const
		{
			return id;
		}

		bool IsValid() const
		{
			return id != 0;
		}
		operator bool() const
		{
			return IsValid();
		}
	};


	template<typename... Params>
	class TBroadcast
	{
	protected:
		using Method   = void(Params...);
		using Function = std::function<void(Params...)>;

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
			Ptr instance;
		};

		mutable TArray<RawListener> rawListeners{};
		mutable TArray<PtrListener> ptrListeners{};


	public:
		TBroadcast()                           = default;
		explicit TBroadcast(const TBroadcast&) = default;
		TBroadcast(TBroadcast&&) noexcept      = default;

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
				if constexpr (IsObject<Type>())
				{
					return Bind<Type>(instance->AsPtr(), Move(method));
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
}    // namespace p
