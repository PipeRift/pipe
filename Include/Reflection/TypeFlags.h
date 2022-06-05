// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "TypeTraits.h"


namespace pipe
{
	/** CLASS TRAITS */

	/** Default traits go here */
	struct DefaultTFlags
	{
		enum
		{
			HasMemberSerialize = false,    // If true, serialization functions for this type will be
			                               // contained inside the type as member functions
			HasSingleSerialize = false,    // If true, Serialize() will be used for reading and
			                               // saving instead of Read() and Write()
			HasGlobalSerialize = false,    // DEPRECATED
			HasPostSerialize   = false
		};
	};

	template<typename...>
	struct TFlags : public DefaultTFlags
	{};
}    // namespace pipe


/** Custom traits go here */
#define TYPE_FLAGS(type, ...)                           \
	template<pipe::Derived<type> T>                     \
	struct pipe::TFlags<T> : public pipe::DefaultTFlags \
	{                                                   \
		enum                                            \
		{                                               \
			__VA_ARGS__                                 \
		};                                              \
	};

#define INHERIT_TYPE_FLAGS(type, parent, ...)            \
	template<typename T>                                 \
		requires pipe::Derived<T, type>                  \
	struct pipe::TFlags<T> : public pipe::TFlags<parent> \
	{                                                    \
		enum                                             \
		{                                                \
			__VA_ARGS__                                  \
		};                                               \
	};

#define TEMPLATE_TYPE_FLAGS(type, ...)                        \
	template<typename T>                                      \
	struct pipe::TFlags<type<T>> : public pipe::DefaultTFlags \
	{                                                         \
		enum                                                  \
		{                                                     \
			__VA_ARGS__                                       \
		};                                                    \
	}

#define INHERIT_TEMPLATE_TYPE_FLAGS(type, parent, ...)         \
	template<typename T>                                       \
	struct pipe::TFlags<type<T>> : public pipe::TFlags<parent> \
	{                                                          \
		enum                                                   \
		{                                                      \
			__VA_ARGS__                                        \
		};                                                     \
	}
