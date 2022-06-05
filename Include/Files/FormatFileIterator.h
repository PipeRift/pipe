// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/String.h"
#include "Files/Files.h"
#include "Log.h"


namespace pipe::files
{
	template<typename FileIterator = files::Iterator>
	class FormatFileIterator
	{
	public:
		StringView format;
		FileIterator fileIterator;

		FormatFileIterator() noexcept = default;
		explicit FormatFileIterator(StringView format, const Path& path);

		FormatFileIterator(const FormatFileIterator&) noexcept = default;
		FormatFileIterator(FormatFileIterator&&) noexcept      = default;
		~FormatFileIterator() noexcept                         = default;

		FormatFileIterator& operator=(const FormatFileIterator&) noexcept = default;
		FormatFileIterator& operator=(FormatFileIterator&&) noexcept = default;

		const fs::directory_entry& operator*() const noexcept
		{
			return *fileIterator;
		}

		const fs::directory_entry* operator->() const noexcept
		{
			return fileIterator.operator->();
		}

		FormatFileIterator& operator++()
		{
			FindNext();
			return *this;
		}

		FormatFileIterator& Increment()
		{
			FindNext();
			return *this;
		}

		// other members as required by [input.iterators]:
		bool operator==(const FormatFileIterator& rhs) const noexcept
		{
			return fileIterator == rhs.fileIterator;
		}

		bool operator!=(const FormatFileIterator& rhs) const noexcept
		{
			return fileIterator != rhs.fileIterator;
		}

		bool IsEnd() const noexcept
		{
			return fileIterator._At_end();
		}

	private:
		void FindNext();

		bool IsValidFile(const FileIterator& it);
	};

	template<typename FileIterator>
	inline FormatFileIterator<FileIterator>::FormatFileIterator(StringView format, const Path& path)
	    : format{format}
	{
		if (!files::Exists(path) || !files::IsFolder(path))
		{
			return;
		}
		fileIterator = FileIterator(path);

		// Iterate to first found asset
		if (!IsValidFile(fileIterator))
		{
			FindNext();
		}
	}

	template<typename FileIterator>
	inline void FormatFileIterator<FileIterator>::FindNext()
	{
		static const FileIterator endIt{};
		std::error_code error;

		fileIterator.increment(error);
		// Loop until end or until we find an asset
		while (fileIterator != endIt && !IsValidFile(fileIterator))
		{
			fileIterator.increment(error);
		}
	}

	template<typename FileIterator>
	bool FormatFileIterator<FileIterator>::IsValidFile(const FileIterator& it)
	{
		return it->path().extension() == format;
	}
}    // namespace pipe::files

namespace pipe
{
	using namespace pipe::files;
}
