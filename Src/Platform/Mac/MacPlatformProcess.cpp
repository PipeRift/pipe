// Copyright 2015-2022 Piperift - All rights reserved

#if PLATFORM_MACOS
#	include "Platform/Mac/MacPlatformProcess.h"
#	include "Platform/PlatformMisc.h"
#	include "Strings/String.h"
#	include "Strings/FixedString.h"
#	include "Files/Paths.h"

#	include <mach-o/dyld.h>
#	include <mach/thread_act.h>
#	include <mach/thread_policy.h>


namespace Rift
{
	StringView MacPlatformProcess::GetExecutableFile()
	{
		static String filePath;
		if (filePath.empty())
		{
			TFixedString<PlatformMisc::GetMaxPathLength(), char> rawPath{};
			u32 size{rawPath.size()};
			if (_NSGetExecutablePath(rawPath.data(), &size) != 0)
			{
				// Failed to retrive a path
				return {};
			}

			filePath = Strings::Convert<String>(
			    TStringView<char>{rawPath.data(), Strings::Length(rawPath.data())});
		}
		return filePath;
	}
	StringView MacPlatformProcess::GetExecutablePath()
	{
		return Paths::GetParent(GetExecutableFile());
	}

	StringView MacPlatformProcess::GetBasePath()
	{
		return GetExecutablePath();
	}

	void MacPlatformProcess::ShowFolder(StringView path)
	{
		if (!Files::Exists(path))
		{
			return;
		}

		CFStringRef CFPath = CFStringCreateWithBytes(kCFAllocatorDefault, (const uint8*)path.data(),
		    path.size() * sizeof(TCHAR),
		    sizeof(TCHAR) == 4 ? kCFStringEncodingUTF32LE : kCFStringEncodingUnicode, false);

		extern void MainThreadCall(dispatch_block_t Block, NSString * WaitMode, bool const bWait);
		if (Files::IsFolder(path))
		{
			MainThreadCall(
			    ^{
				  [[NSWorkspace sharedWorkspace] selectFile:nil
				                   inFileViewerRootedAtPath:(NSString*)CFPath];
				  CFRelease(CFPath);
			    },
			    NSDefaultRunLoopMode, false);
		}
		else if (Files::IsFile(path))
		{
			MainThreadCall(
			    ^{
				  NSString* Directory = [(NSString*)CFPath stringByDeletingLastPathComponent];
				  [[NSWorkspace sharedWorkspace] selectFile:(NSString*)CFPath
				                   inFileViewerRootedAtPath:Directory];
				  CFRelease(CFPath);
			    },
			    NSDefaultRunLoopMode, false);
		}
	}
}
}    // namespace Rift
#endif
