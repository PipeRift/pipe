// Copyright 2015-2021 Piperift - All rights reserved

#include "Files/Files.h"
#include "Files/Paths.h"
#include "Profiler.h"


namespace Rift::Files
{
	bool LoadJsonFile(const Path& path, Json& result)
	{
		ZoneScopedNC("LoadJsonFile", 0xBB45D1);

		if (!Exists(path) || !IsFile(path))
		{
			return false;
		}

		std::ifstream file(path);
		result = {};
		try
		{
			file >> result;
		}
		catch (nlohmann::detail::exception& ex)
		{
			Log::Error("Failed to parse json asset:\n{}", ex.what());
			return false;
		}
		return true;
	}

	bool SaveJsonFile(const Path& path, const Json& data, i32 indent)
	{
		ZoneScopedNC("SaveJsonFile", 0xBB45D1);

		if (!IsFile(path))
		{
			return false;
		}

		std::ofstream file(path);
		if (indent >= 0)
			file << std::setw(indent) << data;
		else
			file << data;

		return true;
	}


	bool LoadStringFile(const Path& path, String& result)
	{
		ZoneScopedNC("LoadStringFile", 0xBB45D1);

		if (!Exists(path) || !IsFile(path))
		{
			return false;
		}

		std::ifstream file(path);

		// Clean string and reserve it
		result = {};
		file.seekg(0, std::ios::end);
		result.reserve(i32(file.tellg()));
		file.seekg(0, std::ios::beg);

		// Improve by avoiding this copy from std::string to String
		std::string str{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
		result.assign(str.data(), str.size());

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

	bool SaveStringFile(const Path& path, const String& data)
	{
		ZoneScopedNC("SaveStringFile", 0xBB45D1);

		if (!IsFile(path))
		{
			return false;
		}

		std::ofstream file(path);
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


	/** String API */

	bool LoadJsonFile(const String& path, Json& result)
	{
		return LoadJsonFile(Paths::FromString(path), result);
	}

	bool SaveJsonFile(const String& path, const Json& data, i32 indent)
	{
		return SaveJsonFile(Paths::FromString(path), data, indent);
	}

	bool LoadStringFile(const String& path, String& result)
	{
		return LoadStringFile(Paths::FromString(path), result);
	}

	bool SaveStringFile(const String& path, const String& data)
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
