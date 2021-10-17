// Copyright 2015-2021 Piperift - All rights reserved

#include "Files/Files.h"

#include "Files/Paths.h"
#include "Log.h"
#include "Profiler.h"


namespace Rift::Files
{
	bool LoadStringFile(const Path& path, String& result, sizet extraPadding)
	{
		ZoneScopedNC("LoadStringFile", 0xBB45D1);

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

	bool Delete(const Path& path, bool bRemoveIfNotEmpty, bool bLogErrors)
	{
		std::error_code err;
		if (bRemoveIfNotEmpty)
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
		if (ExistsAsFolder(origin))
		{
			const auto stdOptions = static_cast<fs::copy_options>(*(options));
			std::error_code err;
			fs::copy(origin, destination, stdOptions, err);
			return !err;
		}
		return false;
	}

	bool Move(const Path& origin, const Path& destination)
	{
		if (ExistsAsFolder(origin))
		{
			std::error_code err;
			fs::rename(origin, destination, err);
			return !err;
		}
		return false;
	}


	/** String API */

	bool LoadStringFile(const String& path, String& result, sizet extraPadding)
	{
		return LoadStringFile(Paths::FromString(path), result, extraPadding);
	}

	bool SaveStringFile(const String& path, StringView data)
	{
		return SaveStringFile(Paths::FromString(path), data);
	}

	bool Exists(const String& path)
	{
		return Exists(Paths::FromString(path));
	}

	bool IsFolder(const String& path)
	{
		return IsFolder(Paths::FromString(path));
	}

	bool IsFile(const String& path)
	{
		return IsFile(Paths::FromString(path));
	}
}    // namespace Rift::Files
