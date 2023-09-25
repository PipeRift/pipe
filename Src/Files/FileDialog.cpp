// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Files/FileDialog.h"

#include "Pipe/Files/Paths.h"

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

	String SelectFileDialog(StringView title, StringView defaultPath,
	    const TArray<DialogFileFilter>& filters, bool alwaysShowDefaultPath)
	{
		pfd::opt options{};
		if (alwaysShowDefaultPath)
		{
			options = options | pfd::opt::force_path;
		}
		pfd::open_file dialog(
		    std::string{title}, std::string{defaultPath}, ParseFilters(filters), options);

		std::vector<std::string> files = dialog.result();
		if (files.size() > 0)
		{
			return String{files[0]};
		}
		return {};
	}

	void SelectFilesDialog(StringView title, StringView defaultPath, TArray<String>& outFiles,
	    const TArray<DialogFileFilter>& filters, bool alwaysShowDefaultPath)
	{
		pfd::opt options = pfd::opt::multiselect;
		if (alwaysShowDefaultPath)
		{
			options = options | pfd::opt::force_path;
		}
		pfd::open_file dialog(
		    std::string{title}, std::string{defaultPath}, ParseFilters(filters), options);

		std::vector<std::string> files = dialog.result();
		outFiles.Resize(i32(files.size()));
		for (u32 i = 0; i < files.size(); ++i)
		{
			outFiles[i] = Move(String{files[0]});
		}
	}

	String SelectFolderDialog(StringView title, StringView defaultPath, bool alwaysShowDefaultPath)
	{
		pfd::opt options{};
		if (alwaysShowDefaultPath)
		{
			options = options | pfd::opt::force_path;
		}
		pfd::select_folder dialog{std::string{title}, std::string{defaultPath}, options};
		return String{dialog.result()};
	}

	String SaveFileDialog(StringView title, StringView defaultPath,
	    const TArray<DialogFileFilter>& filters, bool alwaysShowDefaultPath, bool confirmOverwrite)
	{
		pfd::opt options{};
		if (alwaysShowDefaultPath)
		{
			options = options | pfd::opt::force_path;
		}
		pfd::save_file dialog{
		    std::string{title}, std::string{defaultPath}, ParseFilters(filters), options};
		String path{dialog.result()};
		p::ReplaceExtension(path, "rf");
		return path;
	}
}    // namespace p::files
