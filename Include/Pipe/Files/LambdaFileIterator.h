// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Files/Files.h"


namespace p
{
	template<typename Iterator = PathIterator>
	class LambdaFileIterator
	{
	public:
		TFunction<bool(StringView)> callback;
		Iterator fileIterator;


		LambdaFileIterator() noexcept = default;
		explicit LambdaFileIterator(StringView path, TFunction<bool(StringView)> callback);

		LambdaFileIterator(const LambdaFileIterator&) noexcept = default;
		LambdaFileIterator(LambdaFileIterator&&) noexcept      = default;
		~LambdaFileIterator() noexcept                         = default;

		LambdaFileIterator& operator=(const LambdaFileIterator&) noexcept = default;
		LambdaFileIterator& operator=(LambdaFileIterator&&) noexcept      = default;

		const fs::directory_entry& operator*() const noexcept
		{
			return *fileIterator;
		}

		const fs::directory_entry* operator->() const noexcept
		{
			return fileIterator.operator->();
		}

		LambdaFileIterator& operator++()
		{
			FindNext();
			return *this;
		}

		LambdaFileIterator& Increment()
		{
			FindNext();
			return *this;
		}

		// other members as required by [input.iterators]:
		bool operator==(const LambdaFileIterator& rhs) const noexcept
		{
			return fileIterator == rhs.fileIterator;
		}

		bool operator!=(const LambdaFileIterator& rhs) const noexcept
		{
			return fileIterator != rhs.fileIterator;
		}

		bool IsEnd() const noexcept
		{
			return fileIterator._At_end();
		}

	private:
		void FindNext();
	};

	template<typename Iterator>
	inline LambdaFileIterator<Iterator>::LambdaFileIterator(
	    StringView path, TFunction<bool(StringView)> callback)
	    : callback{p::Move(callback)}
	{
		if (!Exists(path) || !IsFolder(path))
		{
			return;
		}
		fileIterator = Iterator(ToSTDPath(path));

		// Iterate to first found asset
		if (!callback(ToString(fileIterator->path())))
		{
			FindNext();
		}
	}

	template<typename Iterator>
	inline void LambdaFileIterator<Iterator>::FindNext()
	{
		static const Iterator endIt{};
		std::error_code error;

		fileIterator.increment(error);
		// Loop until end or until we find an asset
		while (fileIterator != endIt && !callback(ToString(fileIterator->path())))
		{
			fileIterator.increment(error);
		}
	}
}    // namespace p
