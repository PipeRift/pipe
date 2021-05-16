// Copyright 2015-2021 Piperift - All rights reserved
#pragma once


namespace Rift
{
	/** CLASS TRAITS */

	/** Default traits go here */
	template <typename T>
	struct BaseTypeFlags
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

/** Custom traits go here */
#define DEFINE_TYPE_FLAGS(type, ...)                          \
	template <>                                               \
	struct TypeFlags<type> : public Rift::BaseTypeFlags<type> \
	{                                                         \
		enum                                                  \
		{                                                     \
			__VA_ARGS__                                       \
		};                                                    \
	}
#define DEFINE_TEMPLATE_TYPE_FLAGS(type, ...)                       \
	template <typename T>                                           \
	struct TypeFlags<type<T>> : public Rift::BaseTypeFlags<type<T>> \
	{                                                               \
		enum                                                        \
		{                                                           \
			__VA_ARGS__                                             \
		};                                                          \
	}

	template <typename T>
	struct TypeFlags : public BaseTypeFlags<T>
	{};
}    // namespace Rift
