// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Core/Hash.h"
#include "Core/Platform.h"
#include "Core/StringView.h"
#include "Reflection/Builders/NativeTypeBuilder.h"
#include "Serialization/SerializationFwd.h"

#include <filesystem>


namespace p::files
{
	namespace fs = std::filesystem;

	using Path     = fs::path;
	using PathView = TStringView<Path::value_type>;

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

}    // namespace p::files


namespace p
{
	using namespace p::files;


	template<>
	struct Hash<Path>
	{
		sizet operator()(const Path& path) const
		{
			return GetStringHash(path.c_str());
		}
	};

	CORE_API void Read(p::Reader& ct, p::Path& value);
	CORE_API void Write(p::Writer& ct, const p::Path& value);
}    // namespace p

REFLECT_NATIVE_TYPE(p::Path);


// TODO: Finish implementation of formatting of Paths
/*template <>
struct fmt::formatter<p::Path> : public fmt::formatter<p::PathView>
{
    template <typename FormatContext>
    auto format(const p::Path& path, FormatContext& ctx)
    {
        return formatter<p::PathView>::format(p::ToPathView(path), ctx);
    }
};*/
