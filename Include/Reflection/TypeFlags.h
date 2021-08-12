// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "TypeTraits.h"


namespace Rift
{
	/** CLASS TRAITS */

	/** Default traits go here */
	template <typename T>
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

	template <typename T>
	struct TFlags : public Rift::DefaultTFlags<T>
	{};


/** Custom traits go here */
#define TYPE_FLAGS(type, ...)                              \
	template <Rift::Derived<type> T>                       \
	struct Rift::TFlags<T> : public Rift::DefaultTFlags<T> \
	{                                                      \
		enum                                               \
		{                                                  \
			__VA_ARGS__                                    \
		};                                                 \
	};

#define INHERIT_TYPE_FLAGS(type, parent, ...)            \
	template <Rift::Derived<type> T>                     \
	struct Rift::TFlags<T> : public Rift::TFlags<parent> \
	{                                                    \
		enum                                             \
		{                                                \
			__VA_ARGS__                                  \
		};                                               \
	};

#define TEMPLATE_TYPE_FLAGS(type, ...)                                 \
	template <typename T>                                              \
	struct Rift::TFlags<type<T>> : public Rift::DefaultTFlags<type<T>> \
	{                                                                  \
		enum                                                           \
		{                                                              \
			__VA_ARGS__                                                \
		};                                                             \
	}

#define INHERIT_TEMPLATE_TYPE_FLAGS(type, parent, ...)         \
	template <typename T>                                      \
	struct Rift::TFlags<type<T>> : public Rift::TFlags<parent> \
	{                                                          \
		enum                                                   \
		{                                                      \
			__VA_ARGS__                                        \
		};                                                     \
	}
}    // namespace Rift
