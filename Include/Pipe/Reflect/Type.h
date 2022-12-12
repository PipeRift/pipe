// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/EnumFlags.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Reflect/TypeId.h"


namespace p
{
	enum class TypeCategory : u8
	{
		None   = 0,
		Native = 1 << 0,
		Enum   = 1 << 1,
		Data   = 1 << 2,
		Struct = 1 << 3,
		Class  = 1 << 4,
		All    = Native | Enum | Data | Struct | Class
	};
	P_DEFINE_FLAG_OPERATORS(TypeCategory)


	/** Smallest reflection type */
	class PIPE_API Type
	{
	protected:
		TypeId id;
		TypeCategory category;
		sizet size = 0;

#pragma warning(push)
#pragma warning(disable:4251)
		StringView name;
#pragma warning(pop)


	protected:
		Type(TypeCategory category) : category{category} {}

	public:
		Type(const Type&)            = delete;
		Type& operator=(const Type&) = delete;
		virtual ~Type()              = default;

		TypeId GetId() const
		{
			return id;
		}
		StringView GetName() const;

		TypeCategory GetCategory() const
		{
			return category;
		}
		sizet GetSize() const
		{
			return size;
		}
	};

	// TODO: Move somewhere else
	template<typename T, typename S, typename TRet = CopyConst<T, S>>
	TRet* Cast(S* ptr)
	{
		if constexpr (IsSame<T, S>)
		{
			return ptr;
		}
		else
		{
			static_assert(false, "Invalid cast");
			return nullptr;
		}
	}

	template<typename T, typename S, typename TRet = CopyConst<T, S>>
	TRet* Cast(S* ptr) requires Derived<T, Type>
	{
		static_assert(Derived<T, S, false> || Derived<S, T>, "Casted types are not related!");

		if constexpr (Derived<S, T>)    // Downcasting
		{
			return ptr;
		}
		else if (ptr && HasFlag(ptr->GetCategory(), T::typeCategory))
		{
			return static_cast<TRet*>(ptr);
		}
		return nullptr;
	}
}    // namespace p
