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

		// Reads a type from the current scope
		template <typename T>
		void Serialize(T& val)
		{
			::Write(*this, val);
		}
		template <typename T>
		void Write(T& val)
		{
			::Write(*this, val);
		}

		static constexpr bool IsReading()
		{
			return false;
		}
		static constexpr bool IsWriting()
		{
			return true;
		}
	};

	template <typename T1, typename T2>
	void Write(WriteContext& ct, TPair<T1, T2>& val)
	{
		ct.BeginObject();
		ct.Next("first", v.first);
		ct.Next("second", v.second);
	}

	template <typename T>
	void Write(WriteContext& ct, T& val) requires(
	    bool(TypeFlags<T>::HasMemberSerialize && !TypeFlags<T>::HasSingleSerialize))
	{
		val.Write(ct);
	}

	template <typename T>
	void Write(WriteContext& ct, T& val) requires(IsArray<T>())
	{
		u32 size = val.Size();
		ct.BeginArray(size);
		for (u32 i = 0; i < size; ++i)
		{
			ct.Next(val[i])
		}
	}
}    // namespace Rift::Serl
