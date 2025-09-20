// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "PipePlatform.h"

#include <cstddef>
#include <type_traits>


namespace p
{
	namespace details
	{
		template<class T>
		struct TIsRValueReference : std::false_type
		{};
		template<class T>
		struct TIsRValueReference<T&&> : std::true_type
		{};

		template<class T>
		struct TIsLValueReference : std::false_type
		{};
		template<class T>
		struct TIsLValueReference<T&> : std::true_type
		{};

		template<typename T>
		struct TIsChar : std::false_type
		{};
		template<>
		struct TIsChar<AnsiChar> : std::true_type
		{};
		template<>
		struct TIsChar<WideChar> : std::true_type
		{};
		template<>
		struct TIsChar<const AnsiChar> : std::true_type
		{};
		template<>
		struct TIsChar<const WideChar> : std::true_type
		{};

		// determine whether _Ty can be copy-initialized with {}
		template<typename T, typename = void>
		struct TIsImplicitlyDefaultConstructible : std::false_type
		{};

		template<typename T>
		void TImplicitlyDefaultConstruct(const T&);

		template<typename T>
		struct TIsImplicitlyDefaultConstructible<T,
		    std::void_t<decltype(ImplicitlyDefaultConstruct<T>({}))>> : std::true_type
		{};
	}    // namespace details

	template<typename T>
	concept IsVoid = std::is_void_v<T>;

	template<typename T>
	concept IsStructOrClass = std::is_class_v<T>;

	template<typename One, typename Other>
	concept IsSame = std::is_same_v<One, Other>;

	template<typename Child, typename Base, bool bIncludeSame = true>
	concept Derived = (bIncludeSame && IsSame<Child, Base>) || std::is_base_of_v<Base, Child>;

	template<typename From, typename To>
	concept Convertible = std::is_same_v<From, To> || std::is_convertible_v<From, To>;

	template<typename T>
	concept IsScalar = std::is_scalar_v<T>;

	template<typename T>
	concept Number = std::is_integral_v<T> || std::is_floating_point_v<T>;

	template<typename T>
	concept Integral = std::is_integral_v<T>;
	template<typename T>
	concept SignedIntegral = std::is_integral_v<T> && std::is_signed_v<T>;
	template<typename T>
	concept UnsignedIntegral = std::is_integral_v<T> && std::is_unsigned_v<T>;

	template<typename T>
	concept FloatingPoint = std::is_floating_point_v<T>;

	template<typename T>
	concept IsTrivial = std::is_trivial_v<T>;

	template<typename T>
	concept IsConstructible = std::is_constructible_v<T>;
	template<typename T>
	concept IsTriviallyConstructible = std::is_trivially_constructible_v<T>;

	template<typename T>
	concept IsDefaultConstructible = std::is_default_constructible_v<T>;
	// Determine whether T can be copy-initialized with {}
	template<typename T>
	concept IsImplicitlyDefaultConstructible = details::TIsImplicitlyDefaultConstructible<T>::value;
	template<typename T>
	concept IsTriviallyDefaultConstructible = std::is_trivially_default_constructible_v<T>;

	template<typename T>
	concept IsCopyConstructible = std::is_copy_constructible_v<T>;
	template<typename T>
	concept IsMoveConstructible = std::is_move_constructible_v<T>;
	template<typename T>
	concept IsTriviallyCopyConstructible = std::is_trivially_copy_constructible_v<T>;
	template<typename T>
	concept IsTriviallyMoveConstructible = std::is_trivially_move_constructible_v<T>;

	template<typename T>
	concept IsTriviallyCopyable = std::is_trivially_copyable_v<T>;

	template<typename To, typename From>
	concept IsConvertible = std::is_convertible_v<To, From>;

	template<typename To, typename From>
	concept IsAssignable = std::is_assignable_v<To, From>;
	template<typename To, typename From>
	concept IsTriviallyAssignable = std::is_trivially_assignable_v<To, From>;
	template<typename T>
	concept IsCopyAssignable = std::is_copy_assignable_v<T>;
	template<typename T>
	concept IsMoveAssignable = std::is_move_assignable_v<T>;
	template<typename T>
	concept IsTriviallyCopyAssignable = std::is_trivially_copy_assignable_v<T>;
	template<typename T>
	concept IsTriviallyMoveAssignable = std::is_trivially_move_assignable_v<T>;

	template<typename T>
	concept IsDestructible = std::is_destructible_v<T>;
	template<typename T>
	concept IsTriviallyDestructible = std::is_trivially_destructible_v<T>;


	template<typename T>
	concept IsAbstract = std::is_abstract_v<T>;

	template<typename T>
	concept IsEmpty = std::is_empty_v<T>;

	template<typename T, sizet size>
	concept IsSmaller = sizeof(T) <
	size;

	template<typename T, sizet size>
	concept IsBigger = sizeof(T) >
	size;

	template<typename T>
	concept IsEnum = std::is_enum_v<T>;

	template<typename T>
	concept IsPointer = std::is_pointer_v<T>;

	template<typename T>
	concept IsMemberPointer = std::is_member_pointer_v<T>;

	template<bool UseT, typename T, typename F>
	using Select = typename std::conditional<UseT, T, F>::type;

	template<typename T>
	concept ShouldPassByValue = sizeof(T) <= sizeof(sizet) && IsCopyConstructible<T>;

	template<typename T>
	using ValueOrRef = typename std::conditional<ShouldPassByValue<T>, T, const T&>::type;

	template<typename T>
	concept IsRValueRef = details::TIsRValueReference<T>::value;
	template<typename T>
	concept IsLValueRef = details::TIsLValueReference<T>::value;

	template<typename T>
	concept IsChar = details::TIsChar<T>::value;


	template<typename T>
	concept HasTypeMember = requires { typename T::Type; };
	template<typename T>
	concept HasItemTypeMember = requires { typename T::ItemType; };

	template<typename T>
	concept HasKeyTypeMember = requires { typename T::KeyType; };
	template<typename T>
	concept HasValueTypeMember = requires { typename T::ValueType; };

	template<typename T>
	using UnderlyingType = typename std::underlying_type<T>::type;

	template<typename T>
	using UnwrapEnum = Select<IsEnum<T>, UnderlyingType<T>, T>;

	template<typename T>
	using Mut = std::remove_const_t<T>;
	template<typename T>
	using Const = std::add_const_t<T>;

	template<typename T>
	concept IsConst = std::is_const_v<T>;
	template<typename T>
	concept IsMutable = (!std::is_const_v<T>);
	template<typename T>
	concept IsVolatile = (!std::is_volatile_v<T>);


	template<typename T, typename Reference>
	struct TCopyConst
	{
		using type = T;
	};
	template<typename T, typename Reference>
	struct TCopyConst<T, const Reference>
	{
		using type = const T;
	};
	template<typename T, typename Reference>
	using CopyConst = typename TCopyConst<T, Reference>::type;


	/** An untyped array of data with compile-time alignment and size derived from another type. */
	template<typename T>
	struct TTypeAsBytes
	{
		using Type = T;
		constexpr Type* AsType()
		{
			return reinterpret_cast<Type*>(this);
		}
		constexpr const Type* AsType() const
		{
			return reinterpret_cast<const Type*>(this);
		}

		alignas(Type) u8 pad[sizeof(Type)];
	};

	template<typename Type, i32 Count>
	struct TTypeAsBytesArray
	{
		TTypeAsBytes<Type> data[Count];

		constexpr Type* Data()
		{
			return reinterpret_cast<Type*>(data);
		}
		constexpr const Type* Data() const
		{
			return reinterpret_cast<const Type*>(data);
		}
	};

	template<typename Type>
	struct TTypeAsBytesArray<Type, 0>
	{
		constexpr Type* Data()
		{
			return nullptr;
		}
		constexpr const Type* Data() const
		{
			return nullptr;
		}
	};

	template<typename T>
	struct TRemoveReference
	{
		using Type = T;
	};
	template<typename T>
	struct TRemoveReference<T&>
	{
		using Type = T;
	};
	template<typename T>
	struct TRemoveReference<T&&>
	{
		using Type = T;
	};
	template<typename T>
	using RemoveReference = typename TRemoveReference<T>::Type;


	template<typename Type>
	struct IArray;

	template<typename Type, u32 InlineCapacity>
	struct TInlineArray;
	template<typename Key, typename Value>
	class TMap;


	template<typename T, typename = int>
	struct HasInlineCapacityMember : std::false_type
	{};
	template<typename T>
	struct HasInlineCapacityMember<T, decltype((void)T::inlineCapacity, 0)> : std::true_type
	{};

	template<typename T>
	constexpr bool IsArray()
	{
		// Check if we are dealing with a TArray
		if constexpr (HasItemTypeMember<T> && HasInlineCapacityMember<T>::value)
		{
			return Derived<T, TInlineArray<typename T::ItemType, T::inlineCapacity>>;
		}
		return false;
	}

	template<typename T>
	inline constexpr bool IsArrayView()
	{
		// Check if we are dealing with a TArray
		if constexpr (HasItemTypeMember<T>)
		{
			return Derived<T, IArray<typename T::ItemType>>;
		}
		return false;
	}

	template<typename T>
	inline constexpr bool IsMap()
	{
		// Check if we are dealing with a TAssetPtr
		if constexpr (HasKeyTypeMember<T> && HasValueTypeMember<T>)
		{
			return IsSame<TMap<typename T::KeyType, typename T::ValueType>, T>;
		}
		return false;
	}
}    // namespace p

#define P_DECLARE_IS_TRIVIAL(T, isTrivial)                                                   \
	namespace std                                                                            \
	{                                                                                        \
		template<>                                                                           \
		struct is_trivial<T> : public std::integral_constant<bool, isTrivial>                \
		{};                                                                                  \
		template<>                                                                           \
		struct is_trivial<const T> : public std::integral_constant<bool, isTrivial>          \
		{};                                                                                  \
		template<>                                                                           \
		struct is_trivial<volatile T> : public std::integral_constant<bool, isTrivial>       \
		{};                                                                                  \
		template<>                                                                           \
		struct is_trivial<const volatile T> : public std::integral_constant<bool, isTrivial> \
		{};                                                                                  \
	}
