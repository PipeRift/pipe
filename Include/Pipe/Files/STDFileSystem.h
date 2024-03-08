// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Hash.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Reflect/Builders/NativeTypeBuilder.h"
#include "Pipe/Serialize/SerializationFwd.h"

#include <filesystem>
#include <format>


namespace p
{
	namespace fs = std::filesystem;

	using Path     = fs::path;
	using PathView = TStringView<Path::value_type>;

	using PathIterator          = fs::directory_iterator;
	using RecursivePathIterator = fs::recursive_directory_iterator;
	using SpaceInfo             = fs::space_info;

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
	P_DEFINE_FLAG_OPERATORS(CopyOptions)


	inline PathView ToPathView(const Path& path)
	{
		return {path.c_str()};
	}

	inline sizet GetHash(const Path& path)
	{
		return GetStringHash(path.c_str());
	}

	PIPE_API void Read(p::Reader& ct, p::Path& value);
	PIPE_API void Write(p::Writer& ct, const p::Path& value);

	PIPE_API PathIterator CreatePathIterator(StringView path);
	PIPE_API RecursivePathIterator CreateRecursivePathIterator(StringView path);
}    // namespace p

P_REFLECT_NATIVE_TYPE(p::Path);
