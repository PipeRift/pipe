// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "TypeTraits.h"


namespace Rift
{
	/** CLASS TRAITS */

	/** Default traits go here */
	template <typename T>
	struct DefaultTypeFlags
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
	struct TypeFlags : public Rift::DefaultTypeFlags<T>
	{};


/** Custom traits go here */
#define TYPE_FLAGS(type, ...)                                    \
	template <Rift::Derived<type> T>                             \
	struct Rift::TypeFlags<T> : public Rift::DefaultTypeFlags<T> \
	{                                                            \
		enum                                                     \
		{                                                        \
			__VA_ARGS__                                          \
		};                                                       \
	};

#define INHERIT_TYPE_FLAGS(type, parent, ...)                  \
	template <Rift::Derived<type> T>                           \
	struct Rift::TypeFlags<T> : public Rift::TypeFlags<parent> \
	{                                                          \
		enum                                                   \
		{                                                      \
			__VA_ARGS__                                        \
		};                                                     \
	};

#define TEMPLATE_TYPE_FLAGS(type, ...)                                       \
	template <typename T>                                                    \
	struct Rift::TypeFlags<type<T>> : public Rift::DefaultTypeFlags<type<T>> \
	{                                                                        \
		enum                                                                 \
		{                                                                    \
			__VA_ARGS__                                                      \
		};                                                                   \
	}

#define INHERIT_TEMPLATE_TYPE_FLAGS(type, parent, ...)               \
	template <typename T>                                            \
	struct Rift::TypeFlags<type<T>> : public Rift::TypeFlags<parent> \
	{                                                                \
		enum                                                         \
		{                                                            \
			__VA_ARGS__                                              \
		};                                                           \
	}
}    // namespace Rift
