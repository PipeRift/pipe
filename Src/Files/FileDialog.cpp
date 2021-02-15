// Copyright 2015-2021 Piperift - All rights reserved

#include "Files/FileDialog.h"
#include "Profiler.h"

#include <portable-file-dialogs.h>


namespace Rift::Dialogs
{
	Path SelectFile(StringView title, const Path& defaultPath, bool bAlwaysShowDefaultPath)
	{
		pfd::opt options{};
		if (bAlwaysShowDefaultPath)
		{
			options = options | pfd::opt::force_path;
		}
		pfd::open_file dialog(std::string{title}, defaultPath.string(), {}, options);

        std::vector<std::string> files = dialog.result();
        if (files.size() > 0)
        {
            return Path{files[0]};
        }
        return {};
	}

	void SelectFiles(StringView title, const Path& defaultPath, TArray<Path>& outFiles,
	    bool bAlwaysShowDefaultPath)
	{
		pfd::opt options = pfd::opt::multiselect;
		if (bAlwaysShowDefaultPath)
		{
			options = options | pfd::opt::force_path;
		}
		pfd::open_file dialog(std::string{title}, defaultPath.string(), {}, options);

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

	// Async version of SelectFile
	// void SelectFile(StringView title, TFunction<String()> callback)
	//{
	// auto& tasks = Context::Get().GetTasks();

	// TaskFlow flow;
	// flow.tasks.RunFlow() auto opener = pfd::open_file(title, )
	//}
}    // namespace Rift
