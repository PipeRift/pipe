// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "AssetData.h"
#include "AssetInfo.h"
#include "AssetManager.h"
#include "Files/FileSystem.h"
#include "Reflection/TypeName.h"
#include "Strings/FixedString.h"
#include "Strings/StringView.h"
#include "TypeTraits.h"



namespace Rift
{
	class BaseAssetPtr
	{
		friend class AssetInfoPropertyWidget;

	protected:
		AssetInfo info;
		mutable Ptr<AssetData> cachedAsset;

	public:
		BaseAssetPtr() = default;
		BaseAssetPtr(AssetInfo info) : info{Move(info)} {}

		AssetInfo GetInfo() const
		{
			return info;
		}
		void SetInfo(AssetInfo newInfo)
		{
			info = Move(newInfo);
			cachedAsset.Reset();
		}
	};


	template <class T>
	class TAssetPtr : public BaseAssetPtr
	{
		static_assert(
		    std::is_base_of<AssetData, T>::value, "AssetPtr type must inherit from AssetData");


	public:
		using ItemType = T;

		TAssetPtr() : BaseAssetPtr() {}

		TAssetPtr(TAssetPtr&& other) noexcept
		{
			MoveFrom(Move(other));
		}
		TAssetPtr(const TAssetPtr& other)
		{
			CopyFrom(other);
		}

		TAssetPtr(Path path) : BaseAssetPtr(path) {}
		TAssetPtr(Name path) : BaseAssetPtr(path) {}
		TAssetPtr(StringView key) : TAssetPtr(Name{key}) {}
		TAssetPtr(const AssetInfo& other) : BaseAssetPtr(other) {}
		TAssetPtr(Ptr<ItemType> asset)
		{
			if (asset)
			{
				info        = asset->GetInfo();
				cachedAsset = asset;
			}
		}

		TAssetPtr& operator=(TAssetPtr&& other) noexcept
		{
			MoveFrom(Move(other));
			return *this;
		}
		TAssetPtr& operator=(const TAssetPtr& other)
		{
			CopyFrom(other);
			return *this;
		}

		/**
		 * Tries to load this asset if it's not already
		 * @returns the loaded asset
		 */
		bool Load() const
		{
			if (IsNull())
			{
				return false;
			}
			else if (!IsValid())
			{
				if (auto manager = AssetManager::Get())
				{
					cachedAsset = manager->Load(info);
				}
			}
			return cachedAsset.IsValid();
		}

		/**
		 * Tries to load this asset and returns a new one if not found
		 * @returns the loaded asset
		 */
		bool LoadOrCreate() const
		{
			if (IsNull())
			{
				return false;
			}
			else if (!IsValid())
			{
				if (auto manager = AssetManager::Get())
				{
					cachedAsset = manager->LoadOrCreate(info, GetType<T>());
				}
			}
			return cachedAsset.IsValid();
		}

		/**
		 * @returns the asset if it's valid and loaded
		 */
		Ptr<T> Get() const
		{
			if (!IsNull() && TryCacheAsset())
			{
				return cachedAsset.Cast<T>();
			}
			return {};
		}

		void Reset()
		{
			info = {};
			cachedAsset.Reset();
		}

		/** @returns true if this can never be pointed towards an asset */
		const bool IsNull() const
		{
			return info.IsNull();
		}

		/** @returns true if this asset is loaded */
		const bool IsValid() const
		{
			return !IsNull() && TryCacheAsset();
		}

		inline const Name& GetPath() const
		{
			return info.GetPath();
		}
		inline const String& GetStrPath() const
		{
			return info.GetStrPath();
		}
		inline String GetFilename() const
		{
			return info.GetFilename();
		}

		operator bool() const
		{
			return IsValid();
		};
		Ptr<T> operator*() const
		{
			return Get();
		}
		Ptr<T> operator->() const
		{
			return Get();
		}

		bool Serialize(class Archive& ar, StringView inName)
		{
			return info.Serialize(ar, inName);
		}

	private:
		bool TryCacheAsset() const
		{
			if (cachedAsset)
			{
				return true;
			}

			if (Ptr<AssetManager> manager = AssetManager::Get())
			{
				cachedAsset = manager->GetLoadedAsset(info);
			}
			return cachedAsset.IsValid();
		}

		void MoveFrom(TAssetPtr&& other)
		{
			std::swap(info, other.info);
			std::swap(cachedAsset, other.cachedAsset);
		}

		void CopyFrom(const TAssetPtr& other)
		{
			info        = other.info;
			cachedAsset = other.cachedAsset;
		}
	};

	DEFINE_TEMPLATE_CLASS_TRAITS(TAssetPtr, HasCustomSerialize = true);


	template <typename T>
	inline constexpr bool IsAsset()
	{
		// Check if we are dealing with a TAssetPtr
		if constexpr (HasItemType<T>::value)
		{
			return IsSame<TAssetPtr<typename T::ItemType>, T>;
		}
		return false;
	}

	template <typename T>
	constexpr bool HasType() requires(IsAsset<T>())
	{
		return HasType<typename T::ItemType>();
	}


	// Contains an static fixed string with the name of a TAssetPtr<T>
	template <typename ItemType>
	struct StaticAssetPtrName
	{
		static constexpr auto preffix  = FixedString("TAssetPtr<");
		static constexpr auto suffix   = FixedString(">");
		static constexpr auto itemName = GetTypeName<ItemType>();
		static constexpr FixedString<itemName.size()> fixedItemName{itemName};

		static constexpr auto name = preffix + fixedItemName + suffix;
	};

	template <typename T>
	constexpr StringView GetFullTypeName() requires(IsAsset<T>())
	{
		return StaticAssetPtrName<typename T::ItemType>::name;
	}

	template <typename T>
	constexpr StringView GetTypeName() requires(IsAsset<T>())
	{
		return "TAssetPtr";
	}
}    // namespace Rift
