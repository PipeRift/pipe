// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "CoreTypes.h"
#include "Files/Paths.h"
#include "Reflection/Static/NativeType.h"
#include "Strings/Name.h"
#include "Strings/StringView.h"
#include "TypeTraits.h"


namespace Rift
{
	class AssetInfo
	{
	private:
		Name id;


	public:
		CORE_API AssetInfo() : id(Name::None()) {}
		CORE_API AssetInfo(Name id) : id(Move(id)) {}
		CORE_API AssetInfo(const Path& path) : id(Paths::ToString(path)) {}

		/**
		 * @returns true if this can never be pointed towards an asset
		 */
		CORE_API const bool IsNull() const
		{
			return id.IsNone();
		}

		CORE_API const Name& GetPath() const
		{
			return id;
		}
		CORE_API const String& GetStrPath() const
		{
			return id.ToString();
		}
		CORE_API String GetFilename() const
		{
			// TODO: Optimize after Paths are used for the asset system
			return Paths::GetFilename(Paths::FromString(id.ToString()));
		}

		CORE_API bool operator==(const AssetInfo& other) const
		{
			return id == other.id;
		}

		CORE_API bool Serialize(class Archive& ar, StringView name);
	};


	template <>
	struct Hash<AssetInfo>
	{
		sizet operator()(const AssetInfo& k) const
		{
			static const Hash<Name> nameHash{};
			return nameHash(k.GetPath());
		}
	};

	DEFINE_TYPE_FLAGS(AssetInfo, HasMemberSerialize = true);
	REFLECT_NATIVE_TYPE(AssetInfo);
}    // namespace Rift
