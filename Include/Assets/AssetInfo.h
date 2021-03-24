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
	class CORE_API AssetInfo
	{
	protected:
		Name id;


	public:
		AssetInfo() : id(Name::None()) {}
		AssetInfo(Name id) : id(Move(id)) {}
		AssetInfo(const Path& path) : id(Paths::ToString(path)) {}

		/**
		 * @returns true if this can never be pointed towards an asset
		 */
		const bool IsNull() const
		{
			return id.IsNone();
		}

		inline const Name& GetPath() const
		{
			return id;
		}
		inline const String& GetStrPath() const
		{
			return id.ToString();
		}
		inline String GetFilename() const
		{
			// TODO: Optimize after Paths are used for the asset system
			return Paths::GetFilename(Paths::FromString(id.ToString()));
		}

		bool operator==(const AssetInfo& other) const
		{
			return id == other.id;
		}

		bool Serialize(class Archive& ar, StringView name);
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

	DEFINE_CLASS_TRAITS(AssetInfo, HasCustomSerialize = true);
	REFLECT_NATIVE_TYPE(AssetInfo);
}    // namespace Rift
