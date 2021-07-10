// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "Misc/Hash.h"
#include "Platform/Platform.h"
#include "Reflection/Registry/NativeTypeBuilder.h"
#include "Serialization/ContextsFwd.h"

#include <filesystem>


namespace Rift
{
	namespace fs = std::filesystem;

	using Path = fs::path;

	template <>
	struct Hash<Path>
	{
		sizet operator()(const Path& path) const
		{
			return GetStringHash(path.c_str());
		}
	};

	enum class CopyOptions
	{
		SkipExisting = i32(fs::copy_options::skip_existing),
		Overwrite =
		    i32(fs::copy_options::overwrite_existing) | i32(fs::copy_options::update_existing),
		Recursive = i32(fs::copy_options::recursive),

		CopySymlinks = i32(fs::copy_options::copy_symlinks),
		SkipSymlinks = i32(fs::copy_options::skip_symlinks),

		DirectoriesOnly = i32(fs::copy_options::directories_only),
		CreateSymLinks  = i32(fs::copy_options::create_symlinks),
		CreateHardLinks = i32(fs::copy_options::create_hard_links)
	};

	inline i32 operator*(CopyOptions value)
	{
		return static_cast<i32>(value);
	}
	inline CopyOptions operator|(CopyOptions one, CopyOptions other)
	{
		return static_cast<CopyOptions>(*one | *other);
	}
	inline CopyOptions& operator|=(CopyOptions& one, CopyOptions other)
	{
		one = one | other;
		return one;
	}


	namespace Serl
	{
		CORE_API void Read(Serl::ReadContext& ct, Path& value);
		CORE_API void Write(Serl::WriteContext& ct, const Path& value);
	}    // namespace Serl

	REFLECT_NATIVE_TYPE(Path);
}    // namespace Rift
