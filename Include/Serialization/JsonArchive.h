// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Serialization/Archive.h"


namespace Rift
{
	class JsonArchive : public Archive
	{
		Json baseData;
		TArray<Json*> depthData;

		const bool bBeautify = false;

	public:
		// Save Constructor
		JsonArchive(const bool bBeautify = true) : Archive(false), bBeautify{bBeautify} {}

		// Load constructor
		JsonArchive(const Json& data) : Archive(true), baseData(data), depthData{}, bBeautify{false}
		{}

		String GetDataString() const
		{
			return baseData.dump(GetIndent());
		}
		const Json& GetData() const
		{
			return baseData;
		}

		i32 GetIndent() const
		{
			return bBeautify ? 2 : -1;
		}

	private:
		void Serialize(StringView name, bool& val) override;

		void Serialize(StringView name, u8& val) override;

		void Serialize(StringView name, i32& val) override;

		void Serialize(StringView name, u32& val) override;

		void Serialize(StringView name, float& val) override;

		void Serialize(StringView name, String& val) override;

		void Serialize(StringView name, Json& val) override;

		Json& Data()
		{
			return !depthData.IsEmpty() ? *depthData.Last() : baseData;
		}

		void BeginObject(StringView name) override
		{
			depthData.Add(&Data()[name.data()]);
		}

		void BeginObject(u32 index) override
		{
			depthData.Add(&Data()[index]);
		}

		bool HasObject(StringView name) override
		{
			return Data().find(name) != Data().end();
		}

		bool IsDataValid() override
		{
			return !Data().is_null();
		}

		void EndObject() override
		{
			depthData.RemoveAt(depthData.Size() - 1, false);
		}

		void SerializeArraySize(u32& size) override
		{
			Json& data = Data();
			if (IsLoading())
			{
				size = data.is_null() ? 0 : (u32) data.size();
			}
			else
			{
				if (!data.is_array())
					data = Json::array();
				Json::array_t* ptr = data.get_ptr<Json::array_t*>();
				ptr->reserve(sizet(size));
			}
		};
	};
}    // namespace Rift
