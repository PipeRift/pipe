// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "TypeTraits.h"


namespace Rift
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

	template<typename>
	struct TFlags : public Rift::DefaultTFlags
	{};


/** Custom traits go here */
#define TYPE_FLAGS(type, ...)                           \
	template<typename T>                                \
	requires Derived<T, type>                           \
	struct Rift::TFlags<T> : public Rift::DefaultTFlags \
	{                                                   \
		__VA_OPT__(enum {) __VA_ARGS__ __VA_OPT__(};)   \
	};

#define INHERIT_TYPE_FLAGS(type, parent, ...)            \
	template<typename T>                                 \
	requires Derived<T, type>                            \
	struct Rift::TFlags<T> : public Rift::TFlags<parent> \
	{                                                    \
		__VA_OPT__(enum {) __VA_ARGS__ __VA_OPT__(};)    \
	};

#define TEMPLATE_TYPE_FLAGS(type, ...)                        \
	template<typename T>                                      \
	struct Rift::TFlags<type<T>> : public Rift::DefaultTFlags \
	{                                                         \
		__VA_OPT__(enum {) __VA_ARGS__ __VA_OPT__(};)         \
	}

#define INHERIT_TEMPLATE_TYPE_FLAGS(type, parent, ...)         \
	template<typename T>                                       \
	struct Rift::TFlags<type<T>> : public Rift::TFlags<parent> \
	{                                                          \
		__VA_OPT__(enum {) __VA_ARGS__ __VA_OPT__(};)          \
	}
}    // namespace Rift
