// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Misc/Hash.h"
#include "Platform/Platform.h"
#include "Reflection/Builders/NativeTypeBuilder.h"
#include "Serialization/ContextsFwd.h"
#include "Strings/StringView.h"

#include <filesystem>


namespace Pipe
{
	namespace fs = std::filesystem;

	using Path     = fs::path;
	using PathView = TStringView<Path::value_type>;

	template<>
	struct Hash<Path>
	{
		sizet operator()(const Path& path) const
		{
			return GetStringHash(path.c_str());
		}
	};

	enum class CopyOptions
	{
		None         = i32(fs::copy_options::none),
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


	inline PathView ToPathView(const Path& path)
	{
		return {path.c_str()};
	}

	namespace Serl
	{
		CORE_API void Read(Serl::ReadContext& ct, Path& value);
		CORE_API void Write(Serl::WriteContext& ct, const Path& value);
	}    // namespace Serl

	REFLECT_NATIVE_TYPE(Path);
}    // namespace Pipe


// TODO: Finish implementation of formatting of Paths
/*template <>
struct fmt::formatter<Pipe::Path> : public fmt::formatter<Pipe::PathView>
{
    template <typename FormatContext>
    auto format(const Pipe::Path& path, FormatContext& ctx)
    {
        return formatter<Pipe::PathView>::format(Pipe::ToPathView(path), ctx);
    }
};*/
