// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

// Includes BEFORE Override.h to avoid conflicts with std::malloc and std::free
#include "Export.h"
#include "Memory/Override.h"
#include "Platform/Platform.h"

#include <robin_hood.h>
#include <type_traits>

#include <taskflow/taskflow.hpp>


namespace Rift
{
    // Forward _Arg as movable
	template <class _Ty>
	constexpr std::remove_reference_t<_Ty>&& Move(_Ty&& _Arg) noexcept
	{
		return static_cast<std::remove_reference_t<_Ty>&&>(_Arg);
	}

    // Forward an lvalue as either an lvalue or an rvalue
	template <class _Ty>
	constexpr _Ty&& Forward(std::remove_reference_t<_Ty>& _Arg) noexcept
	{
		return static_cast<_Ty&&>(_Arg);
	}

    // Forward an rvalue as an rvalue
	template <class _Ty>
	constexpr _Ty&& Forward(std::remove_reference_t<_Ty>&& _Arg) noexcept
	{
		static_assert(!std::is_lvalue_reference_v<_Ty>, "Bad Forward call");
		return static_cast<_Ty&&>(_Arg);
	}
}    // namespace Rift
