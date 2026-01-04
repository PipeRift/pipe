// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Core/String.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Export.h"
#include "Pipe/Files/STDFileSystem.h"
#include "PipeArrays.h"
#include "PipePlatform.h"


namespace p
{
	namespace details
	{
		constexpr char separator{'/'};
#if P_PLATFORM_WINDOWS
		constexpr char preferredSeparator{'\\'};
#else
		constexpr char preferredSeparator{'/'};
#endif
		constexpr char dot{'.'};
		constexpr char colon{':'};
	}    // namespace details

	P_API const char* FindRelativeChar(const char* const first, const char* const last);
	P_API const char* FindRelativeChar(
	    const char* const first, const char* const last, const char*& outNameEnd);
	P_API const char* FindFilename(const char* const first, const char* last);
	P_API const char* FindExtension(const char* const first, const char* last);

	// @return root name of a path, or an empty view if missing
	// E.g: "C:\Folder" -> "C:"
	P_API StringView GetRootPathName(const StringView path);

	// @return root path of a path, or an empty view if missing
	// E.g: "C:\Folder" -> "C:\"
	P_API StringView GetRootPath(const StringView path);

	// @return the relative path if it exists, otherwise, an empty view
	// E.g: "C:\Folder\Other" -> "Folder\Other"
	P_API StringView GetRelativePath(const StringView path);

	// @return the path to the parent directory
	// E.g: "/var/tmp/file.txt" -> "/var/tmp"
	// E.g: "/var/tmp/." -> "/var/tmp"
	P_API StringView GetParentPath(StringView path);

	P_API StringView GetFilename(StringView path);
	inline StringView GetFilename(const String& path)
	{
		return GetFilename(StringView{path});
	}
	P_API bool HasFilename(StringView path);
	P_API void RemoveFilename(String& path);
	P_API void RemoveFilename(StringView& path);

	P_API StringView GetStem(StringView path);
	P_API bool HasStem(StringView path);

	P_API StringView GetExtension(StringView path);
	/** Replaces the extension of a path */
	P_API void ReplaceExtension(String& path, StringView newExtension);
	P_API bool HasExtension(StringView path);

	P_API bool IsAbsolutePath(StringView path);
	P_API bool IsRelativePath(StringView path);
	P_API bool IsRemotePath(StringView path);
	P_API bool Exists(StringView path);

	P_API String JoinPaths(StringView base, StringView relative);
	P_API String JoinPaths(StringView base, StringView relative, StringView relative2);
	P_API String JoinPaths(
	    StringView base, StringView relative, StringView relative2, StringView relative3);
	P_API String JoinPaths(TView<StringView> paths);
	P_API void AppendToPath(String& base, StringView other);

	P_API bool AppendPathSeparatorIfNeeded(String& path);

	P_API String ToRelativePath(StringView path);
	P_API String ToAbsolutePath(StringView path);
	P_API String ToRelativePath(StringView path, StringView parent);
	P_API String ToAbsolutePath(StringView path, StringView parent);

	String LexicallyRelative(StringView path, StringView base);
	P_API void SetCanonical(String& path);
	P_API void SetWeaklyCanonical(String& path);

	inline P_API constexpr bool IsSeparator(char c)
	{
		return c == details::separator
#if P_PLATFORM_WINDOWS
		    || c == details::preferredSeparator
#endif
		    ;
	}

	//  For POSIX, IsSeparator() and IsElementSeparator() are identical since
	//  a forward slash is the only valid directory separator and also the only valid
	//  element separator. For Windows, forward slash and back slash are the possible
	//  directory separators, but colon (example: "c:foo") is also an element separator.
	inline P_API constexpr bool IsElementSeparator(char c)
	{
		return c == details::separator
#if P_PLATFORM_WINDOWS
		    || c == details::preferredSeparator || c == details::colon
#endif
		    ;
	}

	P_API String ToString(const Path& path);
	P_API std::filesystem::path ToSTDPath(StringView pathStr);


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

		const char* Peek() const noexcept;
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
		void MakeState(State newState, const char* start, const char* end) noexcept;
		void MakeState(State newState) noexcept;
		const char* GetAfterBack() const noexcept;
		const char* GetBeforeFront() const noexcept;
		/// @return a pointer to the first character after the currently lexed element.
		const char* GetNextTokenStartPos() const noexcept;
		/// @return a pointer to the first character in the currently lexed element.
		const char* GetCurrentTokenStartPos() const noexcept;
		// Consume all consecutive separators
		const char* ConsumeAllSeparators(const char* p, const char* end) const noexcept;
		// Consume exactly N separators, or return nullptr.
		const char* ConsumeNSeparators(const char* p, const char* end, int N) const noexcept;
		const char* ConsumeName(const char* p, const char* end) const noexcept;
		const char* ConsumeDriveLetter(const char* p, const char* end) const noexcept;
		const char* ConsumeNetworkRoot(const char* p, const char* end) const noexcept;
		const char* ConsumeRootName(const char* p, const char* end) const noexcept;
	};
#pragma endregion PathIterator
}    // namespace p
