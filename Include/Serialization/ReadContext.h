// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"
#include "Reflection/ClassTraits.h"
#include "Serialization/Formats/IFormat.h"
#include "Serialization/SerializationTypes.h"
#include "Strings/StringView.h"
#include "Strings/String.h"
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
		virtual ~ReadContext() {}
	};


	CORE_API void EnterScope(ReadContext& ct, StringView name);
	CORE_API void EnterScope(ReadContext& ct, u32 index);
	CORE_API void LeaveScope(ReadContext& ct);

	CORE_API void Read(ReadContext& ct, bool& val);
	CORE_API void Read(ReadContext& ct, u8& val);
	CORE_API void Read(ReadContext& ct, i32& val);
	CORE_API void Read(ReadContext& ct, u32& val);
	CORE_API void Read(ReadContext& ct, float& val);
	CORE_API void Read(ReadContext& ct, String& val);

	template <typename T1, typename T2>
	void Read(ReadContext& ct, TPair<T1, T2>& val)
	{
		ReadScope(ct, "first", v.first);
		ReadScope(ct, "second", v.second);
	}

	template <typename T>
	void Read(ReadContext& ct, T& val) requires(ClassTraits<T>::HasCustomSerialize)
	{
		v.Read(ct, val);
	}

	template <typename T>
	void ReadScope(ReadContext& ct, StringView name, T& val)
	{
		EnterScope(ct, name);
		Read(ct, val);
		LeaveScope(ct);
	}

	//template <typename T>
	//void Read(T& val) requires(IsArray<T>)
	//{
	//	const u32 size = ct.GetScopeSize();
	//	val.Resize(size);
	//	for (u32 i = 0; i < size; ++i)
	//	{
	//		EnterScope(ct, i);
	//		Read(val[i]);
	//		LeaveScope();
	//	}
	//}
}    // namespace Rift::Serl
