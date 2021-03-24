// Copyright 2015-2021 Piperift - All rights reserved
// Some functions are adaptations to StringView from std::filesystem

#include "Files/Paths.h"

#include "Misc/Char.h"


namespace Rift::Paths
{
#if PLATFORM_WINDOWS
	bool _HasDriveLetterPrefix(const TCHAR* const first, const TCHAR* const last)
	{
		// test if [first, last) has a prefix of the form X:
		if (last - first >= 2)
		{
			if (FChar::ToUpper(first[0]) >= 'A' && FChar::ToUpper(first[0]) <= 'Z')
			{
				return first[1] == ':';
			}
		}
		return false;
	}
#endif

	const TCHAR* _FindRootNameEnd(const TCHAR* const first, const TCHAR* const last)
	{
		const sizet len = last - first;
#if PLATFORM_WINDOWS
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

		if (_HasDriveLetterPrefix(first, last))
		{    // check for X: first because it's the most common root-name
			return first + 2;
		}

		if (!IsSlash(first[0]))
		{    // all the other root-names start with a slash; check that first because
			 // we expect paths without a leading slash to be very common
			return first;
		}

		// $ means anything other than a slash, including potentially the end of the input
		if (len >= 4 && IsSlash(first[3]) && (len == 4 || !IsSlash(first[4]))    // \xx\$
		    && ((IsSlash(first[1]) && (first[2] == L'?' || first[2] == L'.'))    // \\?\$ or \\.\$
		           || (first[1] == L'?' && first[2] == L'?')))
		{    // \??\$
			return first + 3;
		}

		if (len >= 3 && IsSlash(first[1]) && !IsSlash(first[2]))
		{    // \\server
			return std::find_if(first + 3, last, IsSlash);
		}

		// no match
		return first;
#else
		// TODO: Make sure this works
		if (len > 2 && IsSlash(first[0]) && IsSlash(first[1]) && !IsSlash(first[2]) &&
		    std::isprint(first[2]))
		{
			const TCHAR* c = first + 3;
			while (c <= last)
			{
				if (IsSlash(*c))
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

	const TCHAR* FindRelativeChar(const TCHAR* const first, const TCHAR* const last)
	{
		// attempt to parse [first, last) as a path and return the start of relative-path
		return std::find_if_not(_FindRootNameEnd(first, last), last, IsSlash);
	}

	StringView GetRootName(const StringView path)
	{
		const auto first = path.data();
		const auto last  = first + path.size();
		return {first, static_cast<size_t>(_FindRootNameEnd(first, last) - first)};
	}

	StringView GetRoot(const StringView path)
	{
		const auto first = path.data();
		const auto last  = first + path.size();
		return {first, static_cast<sizet>(FindRelativeChar(first, last) - first)};
	}

	StringView GetRelative(const StringView path)
	{
		const auto first        = path.data();
		const auto last         = first + path.size();
		const auto relativePath = FindRelativeChar(first, last);
		return {relativePath, static_cast<size_t>(last - relativePath)};
	}

	StringView GetParent(const StringView path)
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
		while (relativePath != last && !IsSlash(last[-1]))
		{
			// handle case 2 by removing trailing filename, puts us into case 1
			--last;
		}

		while (relativePath != last && IsSlash(last[-1]))
		{    // handle case 1 by removing trailing slashes
			--last;
		}

		return {first, static_cast<size_t>(last - first)};
	}
}    // namespace Rift::Paths
