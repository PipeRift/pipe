// Copyright 2015-2023 Piperift - All rights reserved
// Some functions are adaptations to StringView from std::filesystem

#include "Pipe/Files/Paths.h"

#include "Pipe/Core/Char.h"
#include "Pipe/Core/PlatformProcess.h"


// References:
// https://github.com/boostorg/filesystem/blob/develop/include/boost/filesystem/path.hpp
// https://github.com/boostorg/filesystem/blob/develop/src/path.cpp
// https://github.com/boostorg/filesystem/blob/develop/test/deprecated_test.cpp


namespace p::files
{
	Path GetBasePath()
	{
		return ToPath(PlatformProcess::GetBasePath());
	}

#if P_PLATFORM_WINDOWS
	bool HasDriveLetterPrefix(const TChar* const first, const TChar* const last)
	{
		// test if [first, last) has a prefix of the form X:
		if (last - first >= 2 && FChar::ToUpper(first[0]) >= 'A' && FChar::ToUpper(first[0]) <= 'Z')
		{
			return first[1] == ':';
		}
		return false;
	}
#endif

	const TChar* FindRootNameEnd(const TChar* const first, const TChar* const last)
	{
		const sizet len = last - first;
#if P_PLATFORM_WINDOWS
		// attempt to parse [first, last) as a path and return the end of root-name if it exists;
		// otherwise, first

		// This is the place in the generic grammar where library implementations have the most
		// freedom. Below are example Windows paths, and what we've decided to do with them:
		// * X:DriveRelative, X:\DosAbsolute
		//   We parse X: as root-name, if and only if \ is present we consider that root-directory
		// * \RootRelative
		//   We parse no root-name, and \ as root-directory
		// * \\server\share
		//   We parse \\server as root-name, \ as root-directory, and share as the first element in
		//   relative-path. Technically, Windows considers all of \\server\share the logical "root",
		//   but for purposes of decomposition we want those split, so that
		//   path(R"(\\server\share)").replace_filename("other_share") is \\server\other_share
		// * \\?\device
		// * \??\device
		// * \\.\device
		//   CreateFile appears to treat these as the same thing; we will set the first three
		//   characters as root-name and the first \ as root-directory. Support for these prefixes
		//   varies by particular Windows version, but for the purposes of path decomposition we
		//   don't need to worry about that.
		// * \\?\UNC\server\share
		//   MSDN explicitly documents the \\?\UNC syntax as a special case. What actually happens
		//   is that the device Mup, or "Multiple UNC provider", owns the path \\?\UNC in the NT
		//   namespace, and is responsible for the network file access. When the user says
		//   \\server\share, CreateFile translates that into
		//   \\?\UNC\server\share to get the remote server access behavior. Because NT treats this
		//   like any other device, we have chosen to treat this as the \\?\ case above.
		if (len < 2)
		{
			return first;
		}

		if (HasDriveLetterPrefix(first, last))
		{    // check for X: first because it's the most common root-name
			return first + 2;
		}

		if (!IsDirectorySeparator(first[0]))
		{    // all the other root-names start with a slash; check that first because
			 // we expect paths without a leading slash to be very common
			return first;
		}

		// $ means anything other than a slash, including potentially the end of the input
		if (len >= 4 && IsDirectorySeparator(first[3])
		    && (len == 4 || !IsDirectorySeparator(first[4]))    // \xx\$
		    && ((IsDirectorySeparator(first[1])
		            && (first[2] == '?' || first[2] == '.'))    // \\?\$ or \\.\$
		        || (first[1] == '?' && first[2] == '?')))
		{    // \??\$
			return first + 3;
		}

		if (len >= 3 && IsDirectorySeparator(first[1]) && !IsDirectorySeparator(first[2]))
		{    // \\server
			return std::find_if(first + 3, last, IsDirectorySeparator);
		}

		// no match
		return first;
#else
		if (len > 2 && IsDirectorySeparator(first[0]) && IsDirectorySeparator(first[1])
		    && !IsDirectorySeparator(first[2]) && std::isprint(first[2]))
		{
			const TChar* c = first + 3;
			while (c <= last)
			{
				if (IsDirectorySeparator(*c))
				{
					return c;
				}
				++c;
			}
			return last;
		}

		// no match
		return first;
#endif
	}

	const TChar* FindRelativeChar(const TChar* const first, const TChar* const last)
	{
		const TChar* nameEnd;
		return FindRelativeChar(first, last, nameEnd);
	}

	const TChar* FindRelativeChar(
	    const TChar* const first, const TChar* const last, const TChar*& outNameEnd)
	{
		outNameEnd = FindRootNameEnd(first, last);
		// attempt to parse [first, last) as a path and return the start of relative-path
		return std::find_if_not(outNameEnd, last, IsDirectorySeparator);
	}

	const TChar* FindFilename(const TChar* const first, const TChar* last)
	{
		// attempt to parse [first, last) as a path and return the start of filename if it exists;
		// otherwise, last
		const auto relativePath = FindRelativeChar(first, last);
		while (relativePath != last && !IsDirectorySeparator(last[-1]))
		{
			--last;
		}
		return last;
	}

	const TChar* FindExtension(const TChar* const first, const TChar* const last)
	{
		const sizet size           = last - first;
		const TChar* filenameFirst = FindFilename(first, last);
		const sizet filenameSize   = last - filenameFirst;

		if (filenameSize > 0u &&
		    // Check for "." and ".." filenames
		    !(filenameFirst[0] == '.'
		        && (filenameSize == 1u || (filenameSize == 2u && filenameFirst[1u] == '.'))))
		{
			const TChar* extensionFirst = last;
			while (extensionFirst > filenameFirst)
			{
				--extensionFirst;
				if (extensionFirst[0] == '.')
					break;
			}

			if (extensionFirst > filenameFirst)
			{
				return extensionFirst;
			}
		}

		return last;
	}

	StringView GetRootPathName(const StringView path)
	{
		const auto first = path.data();
		const auto last  = first + path.size();
		return {first, static_cast<size_t>(FindRootNameEnd(first, last) - first)};
	}

	StringView GetRootPath(const StringView path)
	{
		const auto first = path.data();
		const auto last  = first + path.size();
		return {first, static_cast<sizet>(FindRelativeChar(first, last) - first)};
	}

	StringView GetRelativePath(const StringView path)
	{
		const auto first        = path.data();
		const auto last         = first + path.size();
		const auto relativePath = FindRelativeChar(first, last);
		return {relativePath, static_cast<size_t>(last - relativePath)};
	}

	StringView GetParentPath(const StringView path)
	{
		const auto first        = path.data();
		auto last               = first + path.size();
		const auto relativePath = FindRelativeChar(first, last);
		// case 1: relative-path ends in a directory-separator, remove the separator to remove
		// "magic empty path"
		//  for example: R"(/cat/dog/\//\)"
		// case 2: relative-path doesn't end in a directory-separator, remove the filename and last
		// directory-separator
		//  to prevent creation of a "magic empty path"
		//  for example: "/cat/dog"
		while (relativePath != last && !IsDirectorySeparator(last[-1]))
		{
			// handle case 2 by removing trailing filename, puts us into case 1
			--last;
		}

		while (relativePath != last && IsDirectorySeparator(last[-1]))
		{    // handle case 1 by removing trailing slashes
			--last;
		}

		return {first, static_cast<size_t>(last - first)};
	}

	StringView GetFilename(StringView path)
	{
		// attempt to parse path as a path and return the filename if it exists; otherwise, an empty
		// view
		const TChar* first    = path.data();
		const TChar* last     = first + path.size();
		const TChar* filename = FindFilename(first, last);
		return StringView{filename, static_cast<sizet>(last - filename)};
	}

	bool HasFilename(StringView path)
	{
		return !GetFilename(path).empty();
	}

	StringView GetStem(StringView path)
	{
		const TChar* first       = path.data();
		const TChar* last        = first + path.size();
		const TChar* stemFirst   = FindFilename(first, last);
		const sizet filenameSize = last - stemFirst;

		// Check for "." and ".." filenames
		if (filenameSize > 0u
		    && !(stemFirst[0] == '.'
		         && (filenameSize == 1 || (filenameSize == 2 && stemFirst[1] == '.'))))
		{
			const TChar* stemLast = last;
			while (stemLast > stemFirst)
			{
				--stemLast;
				if (stemLast[0] == '.')
					break;
			}

			if (stemLast > stemFirst)
			{
				return StringView{stemFirst, stemLast};
			}
		}
		return StringView{stemFirst, static_cast<sizet>(last - stemFirst)};
	}

	bool HasStem(StringView path)
	{
		return !GetStem(path).empty();
	}

	StringView GetExtension(StringView path)
	{
		const TChar* first     = path.data();
		const TChar* last      = first + path.size();
		const TChar* extension = FindExtension(first, last);
		return StringView{extension, static_cast<sizet>(last - extension)};
	}

	void ReplaceExtension(String& path, StringView newExtension)
	{
		const TChar* first     = path.data();
		const TChar* last      = first + path.size();
		const TChar* extension = FindExtension(first, last);
		path.erase(extension - first, last - extension);

		if (!newExtension.empty())
		{
			if (newExtension[0] != dot)
				path.push_back(dot);
			path.append(newExtension);
		}
	}

	bool HasExtension(StringView path)
	{
		return !GetExtension(path).empty();
	}

	bool IsAbsolutePath(StringView path)
	{
		// Is there any root path?
		return FindRelativeChar(path.data(), path.data() + path.size()) != path.data();
	}
	bool IsRelativePath(StringView path)
	{
		return !IsAbsolutePath(path);
	}
	bool Exists(StringView path)
	{
		// TODO: Prevent the use of Path with custom implementation
		Path stdPath{path};
		return std::filesystem::exists(stdPath);
	}


	String JoinPaths(StringView base, StringView relative)
	{
		String result{base};
		AppendToPath(result, relative);
		return Move(result);
	}
	String JoinPaths(StringView base, StringView relative, StringView relative2)
	{
		String result{base};
		AppendToPath(result, relative);
		AppendToPath(result, relative2);
		return Move(result);
	}
	String JoinPaths(TSpan<StringView> paths)
	{
		if (paths.IsEmpty()) [[unlikely]]
		{
			return {};
		}
		String result{paths[0]};
		for (i32 i = 1; i < paths.Size(); ++i)
		{
			StringView relative = paths[i];
			AppendToPath(result, relative);
		}
		return Move(result);
	}
	void AppendToPath(String& base, StringView other)
	{
		if (!other.empty())
		{
			// Overlapping relative source? Use a copy
			if (other.data() >= base.data() && other.data() < (base.data() + base.size()))
			    [[unlikely]]
			{
				String rhs(other);
				AppendToPath(base, rhs);
				return;
			}

			const TChar* const otherEnd = other.data() + other.size();
			const TChar* otherRootNameEnd;
			const TChar* otherRootEnd = FindRelativeChar(other.data(), otherEnd, otherRootNameEnd);

			// Is absolute?
			if (otherRootEnd != other.data())
			{
				base.assign(other.data(), other.size());
				return;
			}

			if (!IsDirectorySeparator(*otherRootNameEnd))
			{
				AppendPathSeparatorIfNeeded(base);
			}
			base.append(otherRootNameEnd, otherEnd);
		}
		else if (HasFilename(base))
		{
			base.push_back(preferredSeparator);
		}
	}

	bool AppendPathSeparatorIfNeeded(String& path)
	{
		if (!path.empty() && !IsElementSeparator(*(path.end() - 1)))
		{
			path += preferredSeparator;
			return true;
		}
		return false;
	}


	String GetFilename(const Path& path)
	{
		return String{GetFilename(StringView{ToString(path)})};
	}
	Path ToRelativePath(const Path& path, const Path& parent)
	{
		return fs::relative(path, parent);
	}

	Path ToAbsolutePath(const Path& path, const Path& parent)
	{
		if (path.is_absolute())
		{
			return path;
		}
		return parent / path;
	}


	String ToString(const Path& path)
	{
		return path.string<TChar, std::char_traits<TChar>, STLAllocator<TChar>>();
	}

	Path ToPath(StringView pathStr)
	{
		Path path;
		path.assign(pathStr);
		return path;
	}
}    // namespace p::files
