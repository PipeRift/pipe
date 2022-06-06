// Copyright 2015-2022 Piperift - All rights reserved

#include "PFiles/Files.h"

#include "PCore/Log.h"
#include "PCore/Profiler.h"
#include "PFiles/Paths.h"

#include <fstream>


namespace p::files
{
	bool LoadStringFile(const Path& path, String& result, sizet extraPadding)
	{
		ZoneScoped;

		if (!Exists(path) || !IsFile(path))
		{
			return false;
		}

		// Clean string and reserve it
		result = {};

		std::ifstream file(path);
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

	void CreateFolder(const Path& path, bool bRecursive)
	{
		if (IsFolder(path) && !Exists(path))
		{
			if (bRecursive)
			{
				CreateFolder(path.parent_path(), bRecursive);
			}
			fs::create_directory(path);
		}
	}

	bool SaveStringFile(const Path& path, StringView data)
	{
		ZoneScopedNC("SaveStringFile", 0xBB45D1);

		if (!IsFile(path))
		{
			return false;
		}

		std::basic_ofstream<TChar> file(path);
		file.write(data.data(), data.size());
		file.close();
		return true;
	}

	bool Delete(const Path& path, bool bExcludeIfNotEmpty, bool bLogErrors)
	{
		std::error_code err;
		if (bExcludeIfNotEmpty)
		{
			fs::remove_all(path, err);
		}
		else
		{
			fs::remove(path, err);
		}
		if (err && bLogErrors)
		{
			Log::Warning(err.message());
		}
		return bool(err);
	}

	Iterator CreateIterator(const Path& path)
	{
		if (!IsFolder(path))
		{
			return {};
		}
		return Iterator{path};
	}

	RecursiveIterator CreateRecursiveIterator(const Path& path)
	{
		if (!Exists(path) || !IsFolder(path))
		{
			return {};
		}
		return RecursiveIterator{path};
	}


	bool Exists(const Path& path)
	{
		return fs::exists(path);
	}

	bool IsFolder(const Path& path)
	{
		return !path.empty() && !path.has_extension();
	}

	bool IsFile(const Path& path, bool /*bCheckOnDisk*/)
	{
		return !path.empty() && path.has_extension();
	}

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

	bool Copy(const Path& origin, const Path& destination, CopyOptions options)
	{
		if (IsFolder(origin) && ExistsAsFolder(origin))
		{
			const auto stdOptions = static_cast<fs::copy_options>(*(options));
			std::error_code err;
			fs::copy(origin, destination, stdOptions, err);
			return !err;
		}
		else if (IsFile(origin) && ExistsAsFile(origin))
		{
			const auto stdOptions = static_cast<fs::copy_options>(*(options));
			std::error_code err;
			fs::copy_file(origin, destination, stdOptions, err);
			return !err;
		}
		return false;
	}

	bool Move(const Path& origin, const Path& destination)
	{
		return Rename(origin, destination);
	}

	bool Rename(const Path& origin, const Path& destination)
	{
		std::error_code err;
		fs::rename(origin, destination, err);
		return !err;
	}


	/** String API */

	bool LoadStringFile(const String& path, String& result, sizet extraPadding)
	{
		return LoadStringFile(FromString(path), result, extraPadding);
	}

	bool SaveStringFile(const String& path, StringView data)
	{
		return SaveStringFile(FromString(path), data);
	}

	bool Exists(const String& path)
	{
		return Exists(FromString(path));
	}

	bool IsFolder(const String& path)
	{
		return IsFolder(FromString(path));
	}

	bool IsFile(const String& path)
	{
		return IsFile(FromString(path));
	}
}    // namespace p::files
