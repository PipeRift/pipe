// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/String.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Export.h"
#include "Pipe/Files/STDFileSystem.h"
#include "PipeArrays.h"


namespace p
{
	constexpr TChar separator{'/'};
#if P_PLATFORM_WINDOWS
	constexpr TChar preferredSeparator{'\\'};
#else
	constexpr TChar preferredSeparator{'/'};
#endif
	constexpr TChar dot{'.'};
	constexpr TChar colon{':'};


	PIPE_API const TChar* FindRelativeChar(const TChar* const first, const TChar* const last);
	PIPE_API const TChar* FindRelativeChar(
	    const TChar* const first, const TChar* const last, const TChar*& outNameEnd);
	PIPE_API const TChar* FindFilename(const TChar* const first, const TChar* last);
	PIPE_API const TChar* FindExtension(const TChar* const first, const TChar* last);

	// @return root name of a path, or an empty view if missing
	// E.g: "C:\Folder" -> "C:"
	PIPE_API StringView GetRootPathName(const StringView path);

	// @return root path of a path, or an empty view if missing
	// E.g: "C:\Folder" -> "C:\"
	PIPE_API StringView GetRootPath(const StringView path);

	// @return the relative path if it exists, otherwise, an empty view
	// E.g: "C:\Folder\Other" -> "Folder\Other"
	PIPE_API StringView GetRelativePath(const StringView path);

	// @return the path to the parent directory
	// E.g: "/var/tmp/file.txt" -> "/var/tmp"
	// E.g: "/var/tmp/." -> "/var/tmp"
	PIPE_API StringView GetParentPath(StringView path);

	PIPE_API StringView GetFilename(StringView path);
	inline StringView GetFilename(const String& path)
	{
		return GetFilename(StringView{path});
	}
	PIPE_API bool HasFilename(StringView path);
	PIPE_API void RemoveFilename(String& path);
	PIPE_API void RemoveFilename(StringView& path);

	PIPE_API StringView GetStem(StringView path);
	PIPE_API bool HasStem(StringView path);

	PIPE_API StringView GetExtension(StringView path);
	/** Replaces the extension of a path */
	PIPE_API void ReplaceExtension(String& path, StringView newExtension);
	PIPE_API bool HasExtension(StringView path);

	PIPE_API bool IsAbsolutePath(StringView path);
	PIPE_API bool IsRelativePath(StringView path);
	PIPE_API bool IsRemotePath(StringView path);
	PIPE_API bool Exists(StringView path);

	PIPE_API String JoinPaths(StringView base, StringView relative);
	PIPE_API String JoinPaths(StringView base, StringView relative, StringView relative2);
	PIPE_API String JoinPaths(
	    StringView base, StringView relative, StringView relative2, StringView relative3);
	PIPE_API String JoinPaths(TView<StringView> paths);
	PIPE_API void AppendToPath(String& base, StringView other);

	PIPE_API bool AppendPathSeparatorIfNeeded(String& path);

	PIPE_API String ToRelativePath(StringView path);
	PIPE_API String ToAbsolutePath(StringView path);
	PIPE_API String ToRelativePath(StringView path, StringView parent);
	PIPE_API String ToAbsolutePath(StringView path, StringView parent);

	String LexicallyRelative(StringView path, StringView base);
	PIPE_API void SetCanonical(String& path);
	PIPE_API void SetWeaklyCanonical(String& path);

	inline PIPE_API constexpr bool IsSeparator(TChar c)
	{
		return c == separator
#if P_PLATFORM_WINDOWS
		    || c == preferredSeparator
#endif
		    ;
	}

	//  For POSIX, IsSeparator() and IsElementSeparator() are identical since
	//  a forward slash is the only valid directory separator and also the only valid
	//  element separator. For Windows, forward slash and back slash are the possible
	//  directory separators, but colon (example: "c:foo") is also an element separator.
	inline PIPE_API constexpr bool IsElementSeparator(TChar c)
	{
		return c == separator
#if P_PLATFORM_WINDOWS
		    || c == preferredSeparator || c == colon
#endif
		    ;
	}

	PIPE_API String ToString(const Path& path);
	PIPE_API std::filesystem::path ToSTDPath(StringView pathStr);


#pragma region PathIterator
	struct PathIterator
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
		PathIterator(StringView path, State state) noexcept : path(path), state(state) {}

	public:
		PathIterator(StringView path, StringView entry, State state)
		    : path(path), rawEntry(entry), state(state)
		{
			// S cannot be '0' or BeforeBegin.
		}

		static PathIterator CreateBegin(StringView p) noexcept;
		static PathIterator CreateEnd(StringView p) noexcept;

		const TChar* Peek() const noexcept;
		void Increment() noexcept;
		void Decrement() noexcept;

		/** @return a view with the "preferred representation" of the current
		 * element. For example trailing separators are represented as a '.'
		 */
		StringView operator*() const noexcept;
		explicit operator bool() const noexcept;
		PathIterator& operator++() noexcept;
		PathIterator& operator--() noexcept;
		bool operator==(const PathIterator& other) const noexcept;
		bool operator!=(const PathIterator& other) const noexcept;

		bool AtEnd() const noexcept;
		bool InRootDir() const noexcept;
		bool InRootName() const noexcept;
		bool InRootPath() const noexcept;

	private:
		void MakeState(State newState, const TChar* start, const TChar* end) noexcept;
		void MakeState(State newState) noexcept;
		const TChar* GetAfterBack() const noexcept;
		const TChar* GetBeforeFront() const noexcept;
		/// @return a pointer to the first character after the currently lexed element.
		const TChar* GetNextTokenStartPos() const noexcept;
		/// @return a pointer to the first character in the currently lexed element.
		const TChar* GetCurrentTokenStartPos() const noexcept;
		// Consume all consecutive separators
		const TChar* ConsumeAllSeparators(const TChar* p, const TChar* end) const noexcept;
		// Consume exactly N separators, or return nullptr.
		const TChar* ConsumeNSeparators(const TChar* p, const TChar* end, int N) const noexcept;
		const TChar* ConsumeName(const TChar* p, const TChar* end) const noexcept;
		const TChar* ConsumeDriveLetter(const TChar* p, const TChar* end) const noexcept;
		const TChar* ConsumeNetworkRoot(const TChar* p, const TChar* end) const noexcept;
		const TChar* ConsumeRootName(const TChar* p, const TChar* end) const noexcept;
	};
#pragma endregion PathIterator
}    // namespace p
