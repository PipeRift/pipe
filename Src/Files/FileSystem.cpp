// Copyright 2015-2021 Piperift - All rights reserved

#include "Files/FileSystem.h"

#include "Profiler.h"
#include "Serialization/Archive.h"


namespace Rift
{
	bool FileSystem::LoadJsonFile(const Path& path, Json& result)
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

	bool FileSystem::SaveJsonFile(const Path& path, const Json& data, i32 indent)
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


	bool FileSystem::LoadStringFile(const Path& path, String& result)
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

	void FileSystem::CreateFolder(const Path& path, bool bRecursive)
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

	bool FileSystem::SaveStringFile(const Path& path, const String& data)
	{
		ZoneScopedNC("SaveStringFile", 0xBB45D1);

		if (!IsFile(path))
		{
			return false;
		}

		std::ofstream file(path);
		file.write(data.data(), data.size());
		return true;
	}

	bool FileSystem::Delete(const Path& path, bool bRemoveIfNotEmpty, bool bLogErrors)
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
}	 // namespace Rift
