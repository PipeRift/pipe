// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Core/Hash.h"
#include "Core/Platform.h"
#include "Core/StringView.h"
#include "Reflection/Builders/NativeTypeBuilder.h"
#include "Serialization/ContextsFwd.h"

#include <filesystem>


namespace pipe::Files
{
	namespace fs = std::filesystem;

	using Path     = fs::path;
	using PathView = TStringView<Path::value_type>;

	template<>
	struct pipe::Hash<Path>
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


}    // namespace pipe::Files

namespace pipe
{
	using namespace pipe::Files;

	namespace Serl
	{
		CORE_API void Read(pipe::ReadContext& ct, pipe::Path& value);
		CORE_API void Write(pipe::WriteContext& ct, const pipe::Path& value);
	}    // namespace Serl
}    // namespace pipe

REFLECT_NATIVE_TYPE(pipe::Path);


// TODO: Finish implementation of formatting of Paths
/*template <>
struct fmt::formatter<pipe::Path> : public fmt::formatter<pipe::PathView>
{
    template <typename FormatContext>
    auto format(const pipe::Path& path, FormatContext& ctx)
    {
        return formatter<pipe::PathView>::format(pipe::ToPathView(path), ctx);
    }
};*/
