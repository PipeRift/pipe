// Copyright 2015-2021 Piperift - All rights reserved
#pragma once


namespace Rift
{
	/** CLASS TRAITS */

	/** Default traits go here */
	template <typename T>
	struct BaseClassTraits
	{
		enum
		{
			HasCustomSerialize = false,
			HasGlobalSerialize = false,
			HasDetailsWidget   = false,
			HasPostSerialize   = false
		};
	};

/** Custom traits go here */
#define DEFINE_CLASS_TRAITS(type, ...)                      \
	template <>                                             \
	struct ClassTraits<type> : public BaseClassTraits<type> \
	{                                                       \
		enum                                                \
		{                                                   \
			__VA_ARGS__                                     \
		};                                                  \
	}
#define DEFINE_TEMPLATE_CLASS_TRAITS(type, ...)                   \
	template <typename T>                                         \
	struct ClassTraits<type<T>> : public BaseClassTraits<type<T>> \
	{                                                             \
		enum                                                      \
		{                                                         \
			__VA_ARGS__                                           \
		};                                                        \
	}

	template <typename T>
	struct ClassTraits : public BaseClassTraits<T>
	{};
}    // namespace Rift
