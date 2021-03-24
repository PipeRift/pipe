// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Assets/AssetInfo.h"
#include "Assets/AssetManager.h"
#include "Files/FormatFileIterator.h"


namespace Rift
{
	template <bool bRecursive = false>
	class AssetIterator : public FormatFileIterator<Files::Iterator>
	{
	public:
		mutable AssetInfo currentAsset;


		using FormatFileIterator<Files::Iterator>::FormatFileIterator;

		explicit AssetIterator(const Path& path)
		    : FormatFileIterator(AssetManager::assetFormat, path)
		{}

		const AssetInfo& operator*() const noexcept
		{
			currentAsset = {fileIterator->path()};
			return currentAsset;
		}

		const AssetInfo* operator->() const noexcept
		{
			return &operator*();
		}
	};

	template <>
	class AssetIterator<true> : public FormatFileIterator<Files::RecursiveIterator>
	{
	public:
		mutable AssetInfo currentAsset;


		using FormatFileIterator<Files::RecursiveIterator>::FormatFileIterator;

		explicit AssetIterator(const Path& path)
		    : FormatFileIterator(AssetManager::assetFormat, path)
		{}

		const AssetInfo& operator*() const noexcept
		{
			currentAsset = {fileIterator->path()};
			return currentAsset;
		}

		const AssetInfo* operator->() const noexcept
		{
			return &operator*();
		}
	};


	template <bool bRecursive>
	inline AssetIterator<bRecursive> begin(AssetIterator<bRecursive> it) noexcept
	{
		return it;
	}

	template <bool bRecursive>
	inline AssetIterator<bRecursive> end(AssetIterator<bRecursive>) noexcept
	{
		return {};
	}
}    // namespace Rift
