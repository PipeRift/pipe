// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Files/FileDialog.h"

#include "Pipe/Core/Profiler.h"

#include <portable-file-dialogs.h>


namespace p::files
{
	std::vector<std::string> ParseFilters(const TArray<DialogFileFilter>& filters)
	{
		std::vector<std::string> rawFilters;
		rawFilters.reserve(sizet(filters.Size()) * 2);
		for (const DialogFileFilter& filter : filters)
		{
			rawFilters.emplace_back(filter.first);
			rawFilters.emplace_back(filter.second);
		}
		return p::Move(rawFilters);
	}

	Path SelectFileDialog(StringView title, const Path& defaultPath,
	    const TArray<DialogFileFilter>& filters, bool alwaysShowDefaultPath)
	{
		pfd::opt options{};
		if (alwaysShowDefaultPath)
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

	void SelectFilesDialog(StringView title, const Path& defaultPath, TArray<Path>& outFiles,
	    const TArray<DialogFileFilter>& filters, bool alwaysShowDefaultPath)
	{
		pfd::opt options = pfd::opt::multiselect;
		if (alwaysShowDefaultPath)
		{
			options = options | pfd::opt::force_path;
		}
		pfd::open_file dialog(
		    std::string{title}, defaultPath.string(), ParseFilters(filters), options);

		std::vector<std::string> files = dialog.result();
		outFiles.Resize(i32(files.size()));
		for (u32 i = 0; i < files.size(); ++i)
		{
			outFiles[i] = Path{files[0]};
		}
	}

	Path SelectFolderDialog(StringView title, const Path& defaultPath, bool alwaysShowDefaultPath)
	{
		pfd::opt options{};
		if (alwaysShowDefaultPath)
		{
			options = options | pfd::opt::force_path;
		}
		pfd::select_folder dialog(std::string{title}, defaultPath.string(), options);
		return Path{dialog.result()};
	}

	Path SaveFileDialog(StringView title, const Path& defaultPath,
	    const TArray<DialogFileFilter>& filters, bool alwaysShowDefaultPath, bool confirmOverwrite)
	{
		pfd::opt options{};
		if (alwaysShowDefaultPath)
		{
			options = options | pfd::opt::force_path;
		}
		pfd::save_file dialog(
		    std::string{title}, defaultPath.string(), ParseFilters(filters), options);
		Path path = {dialog.result()};
		path.replace_extension("rf");
		return path;
	}
}    // namespace p::files
