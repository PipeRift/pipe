// Copyright 2015-2023 Piperift - All rights reserved
// Some functions are adaptations to StringView from std::filesystem

#include "Pipe/Files/Paths.h"

#include "Pipe/Core/Char.h"
#include "Pipe/Core/Macros.h"
#include "Pipe/Core/PlatformProcess.h"

#include <filesystem>


// References:
// https://github.com/boostorg/filesystem/blob/develop/include/boost/filesystem/path.hpp
// https://github.com/boostorg/filesystem/blob/develop/src/path.cpp
// https://github.com/boostorg/filesystem/blob/develop/test/deprecated_test.cpp


namespace p
{
	bool IsDriveLetter(const TChar c)
	{
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	}

	struct PathParser
	{
		enum class State : u8
		{
			// Zero is a special sentinel value used by default constructed iterators.
			BeforeBegin,
			InRootName,
			InRootDir,
			InFilenames,
			InTrailingSep,
			AtEnd
		};

		const StringView path;
		StringView rawEntry;
		State state;

	private:
		PathParser(StringView p, State state) noexcept : path(p), state(state) {}

	public:
		PathParser(StringView p, StringView e, u8 s)
		    : path(p), rawEntry(e), state(static_cast<State>(s))
		{
			// S cannot be '0' or BeforeBegin.
		}

		static PathParser CreateBegin(StringView p) noexcept
		{
			PathParser parser(p, State::BeforeBegin);
			parser.Increment();
			return parser;
		}

		static PathParser CreateEnd(StringView p) noexcept
		{
			return {p, State::AtEnd};
		}

		const TChar* Peek() const noexcept
		{
			auto tkEnd = GetNextTokenStartPos();
			auto end   = GetAfterBack();
			return tkEnd == end ? nullptr : tkEnd;
		}

		void Increment() noexcept
		{
			const TChar* const end   = GetAfterBack();
			const TChar* const start = GetNextTokenStartPos();
			if (start == end)
			{
				MakeState(State::AtEnd);
				return;
			}

			switch (state)
			{
				case State::BeforeBegin: {
					if (const TChar* tkEnd = ConsumeRootName(start, end))
					{
						MakeState(State::InRootName, start, tkEnd);
						return;
					}
					P_FALLTHROUGH;
				}
				case State::InRootName: {
					const TChar* tkEnd = ConsumeAllSeparators(start, end);
					if (tkEnd)
						MakeState(State::InRootDir, start, tkEnd);
					else
						MakeState(State::InFilenames, start, ConsumeName(start, end));
					break;
				}
				case State::InRootDir:
					MakeState(State::InFilenames, start, ConsumeName(start, end));
					break;

				case State::InFilenames: {
					const TChar* sepEnd = ConsumeAllSeparators(start, end);
					if (sepEnd != end)
					{
						const TChar* tkEnd = ConsumeName(sepEnd, end);
						if (tkEnd)
						{
							MakeState(State::InFilenames, sepEnd, tkEnd);
							break;
						}
					}
					MakeState(State::InTrailingSep, start, sepEnd);
					break;
				}

				case State::InTrailingSep: MakeState(State::AtEnd); break;
				case State::AtEnd: Unreachable();
			}
		}

		void Decrement() noexcept
		{
			const TChar* rEnd   = GetBeforeFront();
			const TChar* rStart = GetCurrentTokenStartPos() - 1;
			if (rStart == rEnd)    // we're decrementing the begin
			{
				MakeState(State::BeforeBegin);
				return;
			}

			switch (state)
			{
				case State::AtEnd: {
					// Try to Consume a trailing separator or root directory first.
					if (const TChar* sepEnd = ConsumeAllSeparators(rStart, rEnd))
					{
						if (sepEnd == rEnd)
						{
							MakeState(State::InRootDir, path.data(), rStart + 1);
							return;
						}
						const TChar* tkStart = ConsumeRootName(sepEnd, rEnd);
						if (tkStart == rEnd)
						{
							MakeState(State::InRootDir, rStart, rStart + 1);
						}
						else
						{
							MakeState(State::InTrailingSep, sepEnd + 1, rStart + 1);
						}
					}
					else
					{
						const TChar* tkStart = ConsumeRootName(rStart, rEnd);
						if (tkStart == rEnd)
						{
							MakeState(State::InRootName, tkStart + 1, rStart + 1);
						}
						else
						{
							tkStart = ConsumeName(rStart, rEnd);
							MakeState(State::InFilenames, tkStart + 1, rStart + 1);
						}
					}
					break;
				}
				case State::InTrailingSep:
					MakeState(State::InFilenames, ConsumeName(rStart, rEnd) + 1, rStart + 1);
					break;
				case State::InFilenames: {
					const TChar* sepEnd = ConsumeAllSeparators(rStart, rEnd);
					if (sepEnd == rEnd)
					{
						MakeState(State::InRootDir, path.data(), rStart + 1);
						break;
					}
					const TChar* tkStart = ConsumeRootName(sepEnd ? sepEnd : rStart, rEnd);
					if (tkStart == rEnd)
					{
						if (sepEnd)
						{
							MakeState(State::InRootDir, sepEnd + 1, rStart + 1);
						}
						else
						{
							MakeState(State::InRootName, tkStart + 1, rStart + 1);
						}
					}
					else
					{
						tkStart = ConsumeName(sepEnd, rEnd);
						MakeState(State::InFilenames, tkStart + 1, sepEnd + 1);
					}
					break;
				}
				case State::InRootDir: MakeState(State::InRootName, path.data(), rStart + 1); break;
				case State::InRootName:
				case State::BeforeBegin: Unreachable();
			}
		}

		/// \brief Return a view with the "preferred representation" of the current
		///   element. For example trailing separators are represented as a '.'
		StringView operator*() const noexcept
		{
			switch (state)
			{
				case State::BeforeBegin:
				case State::AtEnd: return "";
				case State::InRootDir:
					if (rawEntry[0] == '\\')
						return "\\";
					else
						return "/";
				case State::InTrailingSep: return "";
				case State::InRootName:
				case State::InFilenames: return rawEntry;
			}
			Unreachable();
		}

		explicit operator bool() const noexcept
		{
			return state != State::BeforeBegin && state != State::AtEnd;
		}

		PathParser& operator++() noexcept
		{
			Increment();
			return *this;
		}

		PathParser& operator--() noexcept
		{
			Decrement();
			return *this;
		}

		bool AtEnd() const noexcept
		{
			return state == State::AtEnd;
		}

		bool InRootDir() const noexcept
		{
			return state == State::InRootDir;
		}

		bool InRootName() const noexcept
		{
			return state == State::InRootName;
		}

		bool InRootPath() const noexcept
		{
			return InRootName() || InRootDir();
		}

	private:
		void MakeState(State newState, const TChar* start, const TChar* end) noexcept
		{
			state    = newState;
			rawEntry = StringView(start, end - start);
		}
		void MakeState(State newState) noexcept
		{
			state    = newState;
			rawEntry = {};
		}

		const TChar* GetAfterBack() const noexcept
		{
			return path.data() + path.size();
		}

		const TChar* GetBeforeFront() const noexcept
		{
			return path.data() - 1;
		}

		/// \brief Return a pointer to the first character after the currently
		///   lexed element.
		const TChar* GetNextTokenStartPos() const noexcept
		{
			switch (state)
			{
				case State::BeforeBegin: return path.data();
				case State::InRootName:
				case State::InRootDir:
				case State::InFilenames: return &rawEntry.back() + 1;
				case State::InTrailingSep:
				case State::AtEnd: return GetAfterBack();
			}
			Unreachable();
		}

		/// \brief Return a pointer to the first character in the currently lexed
		///   element.
		const TChar* GetCurrentTokenStartPos() const noexcept
		{
			switch (state)
			{
				case State::BeforeBegin:
				case State::InRootName: return &path.front();
				case State::InRootDir:
				case State::InFilenames:
				case State::InTrailingSep: return &rawEntry.front();
				case State::AtEnd: return &path.back() + 1;
			}
			Unreachable();
		}

		// Consume all consecutive separators
		const TChar* ConsumeAllSeparators(const TChar* p, const TChar* end) const noexcept
		{
			if (p == nullptr || p == end || !IsSeparator(*p))
				return nullptr;
			const int Inc = p < end ? 1 : -1;
			p += Inc;
			while (p != end && IsSeparator(*p))
				p += Inc;
			return p;
		}

		// Consume exactly N separators, or return nullptr.
		const TChar* ConsumeNSeparators(const TChar* p, const TChar* end, int N) const noexcept
		{
			const TChar* ret = ConsumeAllSeparators(p, end);
			if (ret == nullptr)
				return nullptr;
			if (p < end)
			{
				if (ret == p + N)
					return ret;
			}
			else
			{
				if (ret == p - N)
					return ret;
			}
			return nullptr;
		}

		const TChar* ConsumeName(const TChar* p, const TChar* end) const noexcept
		{
			const TChar* start = p;
			if (p == nullptr || p == end || IsSeparator(*p))
				return nullptr;
			const int inc = p < end ? 1 : -1;
			p += inc;
			while (p != end && !IsSeparator(*p))
				p += inc;
			if (p == end && inc < 0)
			{
				// Iterating backwards and Consumed all the rest of the input.
				// Check if the start of the string would have been considered
				// a root name.
				const TChar* rootEnd = ConsumeRootName(end + 1, start);
				if (rootEnd)
					return rootEnd - 1;
			}
			return p;
		}

		const TChar* ConsumeDriveLetter(const TChar* p, const TChar* end) const noexcept
		{
			if (p == end)
				return nullptr;
			if (p < end)
			{
				if (p + 1 == end || !IsDriveLetter(p[0]) || p[1] != ':')
					return nullptr;
				return p + 2;
			}
			else
			{
				if (p - 1 == end || !IsDriveLetter(p[-1]) || p[0] != ':')
					return nullptr;
				return p - 2;
			}
		}

		const TChar* ConsumeNetworkRoot(const TChar* p, const TChar* end) const noexcept
		{
			if (p == end)
				return nullptr;
			if (p < end)
				return ConsumeName(ConsumeNSeparators(p, end, 2), end);
			else
				return ConsumeNSeparators(ConsumeName(p, end), end, 2);
		}

		const TChar* ConsumeRootName(const TChar* p, const TChar* end) const noexcept
		{
#if defined(_LIBCPP_WIN32API)
			if (const TChar* ret = ConsumeDriveLetter(p, end))
				return ret;
			if (const TChar* ret = ConsumeNetworkRoot(p, end))
				return ret;
#endif
			return nullptr;
		}
	};

	i32 DetermineLexicalElementCount(PathParser pp)
	{
		i32 count = 0;
		for (; pp; ++pp)
		{
			auto elem = *pp;
			if (elem == "..")
				--count;
			else if (elem != "." && elem != "")
				++count;
		}
		return count;
	}


	static String cachedCWD{};
	void SetCurrentPath(StringView path)
	{
		if (PlatformProcess::SetCurrentWorkingPath(path))
		{
			cachedCWD.assign(path);
		}
	}

	StringView GetCurrentPath()
	{
		cachedCWD = Move(PlatformProcess::GetCurrentWorkingPath());
		return cachedCWD;
	}

	StringView GetBasePath()
	{
		return PlatformProcess::GetBasePath();
	}

	StringView GetUserSettingsPath()
	{
		return PlatformProcess::GetUserSettingsPath();
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
		// attempt to parse [first, last) as a path and return the end of root-name if it
		// exists; otherwise, first

		// This is the place in the generic grammar where library implementations have the most
		// freedom. Below are example Windows paths, and what we've decided to do with them:
		// * X:DriveRelative, X:\DosAbsolute
		//   We parse X: as root-name, if and only if \ is present we consider that
		//   root-directory
		// * \RootRelative
		//   We parse no root-name, and \ as root-directory
		// * \\server\share
		//   We parse \\server as root-name, \ as root-directory, and share as the first element
		//   in relative-path. Technically, Windows considers all of \\server\share the logical
		//   "root", but for purposes of decomposition we want those split, so that
		//   path(R"(\\server\share)").replace_filename("other_share") is \\server\other_share
		// * \\?\device
		// * \??\device
		// * \\.\device
		//   CreateFile appears to treat these as the same thing; we will set the first three
		//   characters as root-name and the first \ as root-directory. Support for these
		//   prefixes varies by particular Windows version, but for the purposes of path
		//   decomposition we don't need to worry about that.
		// * \\?\UNC\server\share
		//   MSDN explicitly documents the \\?\UNC syntax as a special case. What actually
		//   happens is that the device Mup, or "Multiple UNC provider", owns the path \\?\UNC
		//   in the NT namespace, and is responsible for the network file access. When the user
		//   says
		//   \\server\share, CreateFile translates that into
		//   \\?\UNC\server\share to get the remote server access behavior. Because NT treats
		//   this like any other device, we have chosen to treat this as the \\?\ case above.
		if (len < 2)
		{
			return first;
		}

		if (HasDriveLetterPrefix(first, last))
		{    // check for X: first because it's the most common root-name
			return first + 2;
		}

		if (!IsSeparator(first[0]))
		{    // all the other root-names start with a slash; check that first because
			 // we expect paths without a leading slash to be very common
			return first;
		}

		// $ means anything other than a slash, including potentially the end of the input
		if (len >= 4 && IsSeparator(first[3]) && (len == 4 || !IsSeparator(first[4]))    // \xx\$
		    && ((IsSeparator(first[1]) && (first[2] == '?' || first[2] == '.'))    // \\?\$ or \\.\$
		        || (first[1] == '?' && first[2] == '?')))
		{    // \??\$
			return first + 3;
		}

		if (len >= 3 && IsSeparator(first[1]) && !IsSeparator(first[2]))
		{    // \\server
			return std::find_if(first + 3, last, IsSeparator);
		}

		// no match
		return first;
#else
		if (len > 2 && IsSeparator(first[0]) && IsSeparator(first[1]) && !IsSeparator(first[2])
		    && std::isprint(first[2]))
		{
			const TChar* c = first + 3;
			while (c <= last)
			{
				if (IsSeparator(*c))
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
		return std::find_if_not(outNameEnd, last, IsSeparator);
	}

	const TChar* FindFilename(const TChar* const first, const TChar* last)
	{
		// attempt to parse [first, last) as a path and return the start of filename if it
		// exists; otherwise, last
		const auto relativePath = FindRelativeChar(first, last);
		while (relativePath != last && !IsSeparator(last[-1]))
		{
			--last;
		}
		return last;
	}

	const TChar* FindExtension(const TChar* const first, const TChar* const last)
	{
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
		// case 2: relative-path doesn't end in a directory-separator, remove the filename and
		// last directory-separator
		//  to prevent creation of a "magic empty path"
		//  for example: "/cat/dog"
		while (relativePath != last && !IsSeparator(last[-1]))
		{
			// handle case 2 by removing trailing filename, puts us into case 1
			--last;
		}

		while (relativePath != last && IsSeparator(last[-1]))
		{    // handle case 1 by removing trailing slashes
			--last;
		}

		return {first, static_cast<size_t>(last - first)};
	}

	StringView GetFilename(StringView path)
	{
		// attempt to parse path as a path and return the filename if it exists; otherwise, an
		// empty view
		const TChar* first    = path.data();
		const TChar* last     = first + path.size();
		const TChar* filename = FindFilename(first, last);
		return StringView{filename, static_cast<sizet>(last - filename)};
	}

	bool HasFilename(StringView path)
	{
		return !GetFilename(path).empty();
	}

	void RemoveFilename(String& path)
	{
		const TChar* first    = path.data();
		const TChar* last     = first + path.size();
		const TChar* filename = FindFilename(first, last);
		path.erase(static_cast<sizet>(filename - first));
	}

	void RemoveFilename(StringView& path)
	{
		const TChar* first    = path.data();
		const TChar* last     = first + path.size();
		const TChar* filename = FindFilename(first, last);
		path                  = StringView{first, static_cast<sizet>(filename - first)};
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
	String JoinPaths(
	    StringView base, StringView relative, StringView relative2, StringView relative3)
	{
		String result{base};
		AppendToPath(result, relative);
		AppendToPath(result, relative2);
		AppendToPath(result, relative3);
		return Move(result);
	}
	String JoinPaths(TView<StringView> paths)
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

			if (!IsSeparator(*otherRootNameEnd))
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


	String ToRelativePath(StringView path)
	{
		return ToRelativePath(path, GetCurrentPath());
	}

	String ToAbsolutePath(StringView path)
	{
		if (p::IsAbsolutePath(path))
		{
			return String{path};
		}
		return p::JoinPaths(GetCurrentPath(), path);
	}

	String ToRelativePath(StringView path, StringView parent)
	{
		String result{path};
		SetWeaklyCanonical(result);
		String canonicalParent{parent};
		SetWeaklyCanonical(canonicalParent);
		return LexicallyRelative(result, canonicalParent);
	}

	String ToAbsolutePath(StringView path, StringView parent)
	{
		// If absolute, append will return 'path' directly
		String result{parent};
		AppendToPath(result, path);
		return Move(result);
	}

	String LexicallyRelative(StringView path, StringView base)
	{
		{    // perform root-name/root-directory mismatch checks
			auto pp                      = PathParser::CreateBegin(path);
			auto ppBase                  = PathParser::CreateBegin(base);
			auto CheckIterMismatchAtBase = [&]() {
				return pp.state != ppBase.state && (pp.InRootPath() || ppBase.InRootPath());
			};
			if (pp.InRootName() && ppBase.InRootName())
			{
				if (*pp != *ppBase)
					return {};
			}
			else if (CheckIterMismatchAtBase())
			{
				return {};
			}

			if (pp.InRootPath())
				++pp;
			if (ppBase.InRootPath())
				++ppBase;
			if (CheckIterMismatchAtBase())
			{
				return {};
			}
		}

		// Find the first mismatching element
		auto pp     = PathParser::CreateBegin(path);
		auto ppBase = PathParser::CreateBegin(base);
		while (pp && ppBase && pp.state == ppBase.state && *pp == *ppBase)
		{
			++pp;
			++ppBase;
		}

		// If there is no mismatch, return ".".
		if (!pp && !ppBase)
			return ".";

		// Otherwise, determine the number of elements, 'n', which are not dot or
		// dot-dot minus the number of dot-dot elements.
		i32 elemCount = DetermineLexicalElementCount(ppBase);
		if (elemCount < 0)
			return {};

		// if n == 0 and (a == end() || a->empty()), returns "."; otherwise
		if (elemCount == 0 && (pp.AtEnd() || *pp == ""))
			return ".";

		// return a path constructed with 'n' dot-dot elements, followed by the
		// elements of '*this' after the mismatch.
		String tmp;
		while (elemCount--)
			AppendToPath(tmp, "..");
		for (; pp; ++pp)
			AppendToPath(tmp, *pp);
		return tmp;
	}
	void SetCanonical(String& path)
	{
		// TODO: Implement natively
		// https://github.com/boostorg/filesystem/blob/develop/src/operations.cpp#L2407
		path = ToString(std::filesystem::canonical(ToSTDPath(path)));
	}

	void SetWeaklyCanonical(String& path)
	{
		// TODO: Implement natively
		// https://github.com/boostorg/filesystem/blob/develop/src/operations.cpp#L4434
		path = ToString(std::filesystem::weakly_canonical(ToSTDPath(path)));
	}

	String ToString(const Path& path)
	{
		return path.string<TChar, std::char_traits<TChar>, std::allocator<TChar>>();
	}

	Path ToSTDPath(StringView pathStr)
	{
		Path path;
		path.assign(pathStr);
		return path;
	}
}    // namespace p
