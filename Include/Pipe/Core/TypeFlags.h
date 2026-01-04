// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "Pipe/Core/TypeTraits.h"


namespace p
{
	/** CLASS TRAITS */

	/** Default traits go here */
	struct DefaultTFlags
	{
		enum
		{
			HasMemberSerialize = false,    // If true, serialization functions for this type will be
			                               // contained inside the type as member functions
			HasSingleSerialize = false     // If true, Serialize() will be used for reading and
			                               // saving instead of Read() and Write()
		};
	};

	template<typename...>
	struct TFlags : public DefaultTFlags
	{};
}    // namespace p


/** Custom traits go here */
#define TYPE_FLAGS(type, ...)                     \
	template<p::Derived<type> T>                  \
	struct p::TFlags<T> : public p::DefaultTFlags \
	{                                             \
		enum                                      \
		{                                         \
			__VA_ARGS__                           \
		};                                        \
	};

#define INHERIT_TYPE_FLAGS(type, parent, ...)      \
	template<typename T>                           \
	requires p::Derived<T, type>                   \
	struct p::TFlags<T> : public p::TFlags<parent> \
	{                                              \
		enum                                       \
		{                                          \
			__VA_ARGS__                            \
		};                                         \
	};

#define TEMPLATE_TYPE_FLAGS(type, ...)                  \
	template<typename T>                                \
	struct p::TFlags<type<T>> : public p::DefaultTFlags \
	{                                                   \
		enum                                            \
		{                                               \
			__VA_ARGS__                                 \
		};                                              \
	}

#define INHERIT_TEMPLATE_TYPE_FLAGS(type, parent, ...)   \
	template<typename T>                                 \
	struct p::TFlags<type<T>> : public p::TFlags<parent> \
	{                                                    \
		enum                                             \
		{                                                \
			__VA_ARGS__                                  \
		};                                               \
	}
