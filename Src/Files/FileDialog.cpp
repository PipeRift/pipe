// Copyright 2015-2021 Piperift - All rights reserved

#include "Files/FileDialog.h"
#include "Profiler.h"

#include <portable-file-dialogs.h>


namespace Rift::Dialogs
{
	std::vector<std::string> ParseFilters(const TArray<FileFilter>& filters)
	{
		std::vector<std::string> rawFilters;
		rawFilters.reserve(filters.Size() * 2);
		for (const FileFilter& filter : filters)
		{
			rawFilters.push_back(std::string(filter.first));
			rawFilters.push_back(std::string(filter.second));
		}
		return Move(rawFilters);
	}

	Path SelectFile(StringView title, const Path& defaultPath, const TArray<FileFilter>& filters,
	    bool bAlwaysShowDefaultPath)
	{
		pfd::opt options{};
		if (bAlwaysShowDefaultPath)
		{
			options = options | pfd::opt::force_path;
		}
		pfd::open_file dialog(
		    std::string{title}, defaultPath.string(), ParseFilters(filters), options);

		std::vector<std::string> files = dialog.result();
        if (files.size() > 0)
        {
            return Path{files[0]};
        }
        return {};
	}

	void SelectFiles(StringView title, const Path& defaultPath, TArray<Path>& outFiles,
	    const TArray<FileFilter>& filters, bool bAlwaysShowDefaultPath)
	{
		pfd::opt options = pfd::opt::multiselect;
		if (bAlwaysShowDefaultPath)
		{
			options = options | pfd::opt::force_path;
		}
		pfd::open_file dialog(
		    std::string{title}, defaultPath.string(), ParseFilters(filters), options);

		std::vector<std::string> files = dialog.result();
        outFiles.Resize(files.size());
        for(u32 i = 0; i < files.size(); ++i)
        {
            outFiles[i] = Path{files[0]};
        }
	}

	Path SelectFolder(StringView title, const Path& defaultPath, bool bAlwaysShowDefaultPath)
	{
		pfd::opt options{};
		if (bAlwaysShowDefaultPath)
		{
			options = options | pfd::opt::force_path;
		}
		pfd::select_folder dialog(std::string{title}, defaultPath.string(), options);
		return Path{dialog.result()};
	}
}    // namespace Rift
