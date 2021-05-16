// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"
#include "Reflection/TypeFlags.h"
#include "Serialization/Formats/IFormat.h"
#include "Serialization/SerializationTypes.h"
#include "Strings/String.h"
#include "Strings/StringView.h"


namespace Rift::Serl
{
	struct WriteContext
	{
		template <Format format>
		friend struct TFormatWriter;

		Flags flags           = Flags_None;
		Format format         = Format_None;
		IFormatWriter* writer = nullptr;


	private:
		WriteContext() = default;

	public:
		WriteContext(const WriteContext&) = default;
		WriteContext& operator=(const WriteContext&) = default;
		virtual ~WriteContext() {}
	};


	CORE_API bool EnterScope(WriteContext& ct, StringView name);
	CORE_API bool EnterScope(WriteContext& ct, u32 index);
	CORE_API void LeaveScope(WriteContext& ct);

	template <typename T>
	void Write(WriteContext& ct, T& val) requires(IsArray<T>())
	{}

	template <typename T>
	void WriteScope(WriteContext& ct, StringView name, const T& val)
	{
		EnterScope(ct, name);
		Write(ct, val);
		LeaveScope(ct);
	}

	template <typename T>
	bool IsReading() requires(Derived<T, WriteContext>)
	{
		return false;
	}
	template <typename T>
	bool IsWriting() requires(Derived<T, WriteContext>)
	{
		return true;
	}
}    // namespace Rift::Serl
