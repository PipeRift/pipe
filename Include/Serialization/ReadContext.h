// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"
#include "Reflection/ClassTraits.h"
#include "Serialization/IReader.h"
#include "Serialization/SerializationTypes.h"


namespace Rift::Serl
{
	struct ReadContext
	{
		Flags flags           = Flags_None;
		Format format         = Format_None;
		IFormatReader* reader = nullptr;

		ReadContext(IReader& reader) : reader(&reader) {}
		virtual ~ReadContext() {}

		// UNCHECKED helper returning a Reader from a format
		template <Format format>
		FormatReaderBind<format>::Type& GetReader() const;
		{
			return *static_cast<FormatReaderBind<format>::Type*>(reader);
		}
	};

	// Try to read a value directly from the format reader.
	// Intended for internal use.
	template <typename OnReadChild>
	void ReadChild(ReadContext& ct, StringView name, OnReadChild onReadChild)
	{
		switch (ct.format)
		{
			case Format_Json:
				// ct.GetReader<Format_Json>().Read(name, v);
				break;
			default:
				break;
		}
	}

	// Try to read a value directly from the format reader.
	// Intended for internal use.
	template <typename T>
	void ReadFromFormat(ReadContext& ct, StringView name, T& v)
	{
		switch (ct.format)
		{
			case Format_Json:
				ct.GetReader<Format_Json>().Read(name, v);
				break;
			default:
				break;
		}
	}

	void Read(ReadContext& ct, StringView name, bool& v)
	{
		ReadFromFormat(ct, name, v);
	}

	template <typename T1, typename T2>
	void Read(ReadContext& ct, StringView name, TPair<T1, T2>& v)
	{
		Read(ct, "first", v.first);
		Read(ct, "second", v.second);
	}

	template <typename T>
	void Read(ReadContext& ct, StringView name, T& v) requires(ClassTraits<T>::HasCustomSerialize)
	{
		v.Read(ct, name);
	}
}    // namespace Rift::Serl
