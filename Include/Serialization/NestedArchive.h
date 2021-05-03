// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Serialization/Archive.h"


namespace Rift
{
	class NestedArchive : public Archive
	{
		Archive& parent;

	public:
		NestedArchive(Archive& inParent) : Archive(inParent.IsLoading()), parent(parent) {}

	private:
		void Serialize(StringView name, bool& val) override
		{
			parent.Serialize(name, val);
		}

		void Serialize(StringView name, u8& val) override
		{
			parent.Serialize(name, val);
		}

		void Serialize(StringView name, i32& val) override
		{
			parent.Serialize(name, val);
		}

		void Serialize(StringView name, u32& val) override
		{
			parent.Serialize(name, val);
		}

		void Serialize(StringView name, float& val) override
		{
			parent.Serialize(name, val);
		}

		void Serialize(StringView name, String& val) override
		{
			parent.Serialize(name, val);
		}

		void Serialize(StringView name, Json& val) override
		{
			parent.Serialize(name, val);
		}

		void BeginObject(StringView name) override
		{
			parent.BeginObject(name);
		}

		void BeginObject(u32 index) override
		{
			parent.BeginObject(index);
		}

		bool HasObject(StringView name) override
		{
			return parent.HasObject(name);
		}

		bool IsDataValid() override
		{
			return parent.IsDataValid();
		}

		void EndObject() override
		{
			parent.EndObject();
		}

		void SerializeArraySize(u32& size) override
		{
			parent.SerializeArraySize(size);
		};
	};
}    // namespace Rift
