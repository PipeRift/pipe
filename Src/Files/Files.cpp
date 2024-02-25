// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Files/Files.h"

#include "Pipe/Core/Log.h"
#include "Pipe/Files/Paths.h"

#include <fstream>


namespace p::files
{

	bool ExistsAsFile(const Path& path)
	{
		return fs::is_regular_file(path);
	}

	bool ExistsAsFolder(const Path& path)
	{
		return fs::is_directory(path);
	}

	SpaceInfo Space(const Path& target)
	{
		return fs::space(target);
	}


	/** String API */

	bool CreateFolder(StringView path, bool bRecursive)
	{
		if (IsFolder(path) && !Exists(path))
		{
			if (bRecursive)
			{
				CreateFolder(p::GetParentPath(path), bRecursive);
			}
			return fs::create_directory(path);
		}
		return false;
	}

	bool Delete(StringView path, bool bExcludeIfNotEmpty, bool bLogErrors)
	{
		std::error_code err;
		if (bExcludeIfNotEmpty)
		{
			fs::remove_all(ToSTDPath(path), err);
		}
		else
		{
			fs::remove(ToSTDPath(path), err);
		}
		if (err && bLogErrors)
		{
			Warning(err.message());
		}
		return bool(err);
	}

	bool Copy(StringView origin, StringView destination, CopyOptions options)
	{
		if (IsFolder(origin) && ExistsAsFolder(origin))
		{
			const auto stdOptions = static_cast<fs::copy_options>(*(options));
			std::error_code err;
			fs::copy(ToSTDPath(origin), ToSTDPath(destination), stdOptions, err);
			return !err;
		}
		else if (IsFile(origin) && ExistsAsFile(origin))
		{
			const auto stdOptions = static_cast<fs::copy_options>(*(options));
			std::error_code err;
			fs::copy_file(ToSTDPath(origin), ToSTDPath(destination), stdOptions, err);
			return !err;
		}
		return false;
	}

	bool Move(StringView origin, StringView destination)
	{
		return Rename(origin, destination);
	}

	bool Rename(StringView origin, StringView destination)
	{
		std::error_code err;
		fs::rename(ToSTDPath(origin), ToSTDPath(destination), err);
		return !err;
	}

	bool LoadStringFile(StringView path, String& result, sizet extraPadding)
	{
		if (!Exists(path) || !IsFile(path))
		{
			return false;
		}

		// Clean string and reserve it
		result = {};

		std::ifstream file(String{path});
		file.seekg(0, std::ios::end);
		const sizet size = sizet(file.tellg());
		if (size > 0)
		{
			result.reserve(size + extraPadding);

			file.seekg(0, std::ios::beg);
			result.assign(std::istreambuf_iterator<TChar>(file), std::istreambuf_iterator<TChar>());
		}
		return !result.empty();
	}

	bool SaveStringFile(StringView path, StringView data)
	{
		if (!IsFile(path))
		{
			return false;
		}

		std::basic_ofstream<TChar> file(String{path});
		file.write(data.data(), data.size());
		file.close();
		return true;
	}

	bool Exists(StringView path)
	{
		return fs::exists(ToSTDPath(path));
	}

	bool IsFolder(StringView path)
	{
		return !path.empty() && !HasExtension(path);
	}

	bool IsFile(StringView path)
	{
		return !path.empty() && HasExtension(path);
	}
}    // namespace p::files
