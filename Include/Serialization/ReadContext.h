// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"
#include "Reflection/TypeFlags.h"
#include "Serialization/Formats/IFormat.h"
#include "Serialization/SerializationTypes.h"
#include "Strings/String.h"
#include "Strings/StringView.h"
#include "Templates/Tuples.h"


namespace Rift::Serl
{
	struct ReadContext
	{
		template <Format format>
		friend struct TFormatReader;

		Flags flags           = Flags_None;
		Format format         = Format_None;
		IFormatReader* reader = nullptr;


	private:
		ReadContext() = default;

	public:
		ReadContext(const ReadContext&) = default;
		ReadContext& operator=(const ReadContext&) = default;
		virtual ~ReadContext() {}
	};


	CORE_API bool EnterScope(ReadContext& ct, StringView name);
	CORE_API bool EnterScope(ReadContext& ct, u32 index);
	CORE_API void LeaveScope(ReadContext& ct);

	CORE_API void IterateObject(ReadContext& ct, TFunction<void()> callback);
	CORE_API void IterateObject(ReadContext& ct, TFunction<void(const char*)> callback);
	CORE_API bool IsObject(ReadContext& ct);
	CORE_API sizet GetObjectSize(ReadContext& ct);

	CORE_API void IterateArray(ReadContext& ct, TFunction<void()> callback);
	CORE_API void IterateArray(ReadContext& ct, TFunction<void(u32)> callback);
	CORE_API bool IsArray(ReadContext& ct);
	CORE_API sizet GetArraySize(ReadContext& ct);

	CORE_API void Read(ReadContext& ct, bool& val);
	CORE_API void Read(ReadContext& ct, u8& val);
	CORE_API void Read(ReadContext& ct, i32& val);
	CORE_API void Read(ReadContext& ct, u32& val);
	CORE_API void Read(ReadContext& ct, i64& val);
	CORE_API void Read(ReadContext& ct, u64& val);
	CORE_API void Read(ReadContext& ct, float& val);
	CORE_API void Read(ReadContext& ct, double& val);
	CORE_API void Read(ReadContext& ct, StringView& val);
	CORE_API void Read(ReadContext& ct, String& val);

	template <typename T1, typename T2>
	void Read(ReadContext& ct, TPair<T1, T2>& val)
	{
		ReadScope(ct, "first", v.first);
		ReadScope(ct, "second", v.second);
	}

	template <typename T>
	void Read(ReadContext& ct, T& val) requires(bool(TypeFlags<T>::HasMemberSerialize))
	{
		if constexpr (TypeFlags<T>::HasSingleSerialize)
		{
			val.Serialize(ct);
		}
		else
		{
			val.Read(ct);
		}
	}

	// Some types
	template <typename T>
	void Read(ReadContext& ct, T& val) requires(
	    bool(TypeFlags<T>::HasSingleSerialize && !TypeFlags<T>::HasMemberSerialize))
	{
		Serialize(ct, val);
	}

	template <typename T>
	void Read(ReadContext& ct, T& val) requires(IsArray<T>())
	{
		val.Resize(GetArraySize(ct));
		IterateArray(ct, [&ct](u32 index) {
			Read(ct, val[index]);
		});
	}


	template <typename T>
	void ReadScope(ReadContext& ct, StringView name, T& val)
	{
		EnterScope(ct, name);
		Read(ct, val);
		LeaveScope(ct);
	}


	template <typename T>
	bool IsReading() requires(Derived<T, ReadContext>)
	{
		return true;
	}
	template <typename T>
	bool IsWriting() requires(Derived<T, ReadContext>)
	{
		return false;
	}
}    // namespace Rift::Serl
