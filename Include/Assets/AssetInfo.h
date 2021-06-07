// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "CoreTypes.h"
#include "Files/Paths.h"
#include "Reflection/Reflection.h"
#include "Serialization/ContextsFwd.h"
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

		CORE_API void Read(Serl::ReadContext& ct);
		CORE_API void Write(Serl::WriteContext& ct) const;
	};
	REFLECT_NATIVE_TYPE(AssetInfo);


	template <>
	struct Hash<AssetInfo>
	{
		sizet operator()(const AssetInfo& k) const
		{
			static const Hash<Name> nameHash{};
			return nameHash(k.GetPath());
		}
	};

}    // namespace Rift

DEFINE_TYPE_FLAGS(Rift::AssetInfo, HasMemberSerialize = true);
