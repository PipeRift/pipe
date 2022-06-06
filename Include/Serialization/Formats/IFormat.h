// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/Platform.h"
#include "Serialization/SerializationTypes.h"
#include "TypeTraits.h"


namespace p
{
	struct IFormatReader
	{};

	enum CORE_API WriteFlags
	{
		WriteFlags_None              = 0,
		WriteFlags_CacheStringKeys   = 1 << 0,
		WriteFlags_CacheStringValues = 1 << 1,
		WriteFlags_CacheStrings      = WriteFlags_CacheStringKeys | WriteFlags_CacheStringValues
	};

	struct IFormatWriter
	{
	private:
		sizet activeFlags = WriteFlags_None;
		TArray<sizet> flagStack;

	public:
		CORE_API void PushAddFlags(WriteFlags flags)
		{
			flagStack.Add(activeFlags);
			activeFlags |= flags;
		}
		CORE_API void PushRemoveFlags(WriteFlags flags)
		{
			flagStack.Add(activeFlags);
			activeFlags &= ~flags;
		}
		CORE_API void PopFlags()
		{
			activeFlags = flagStack.Last();
			flagStack.RemoveAt(flagStack.Size() - 1);
		}

		CORE_API sizet GetFlags() const
		{
			return activeFlags;
		}
	};


	template<SerializeFormat format>
	struct FormatBind
	{
		using Reader = void;
		using Writer = void;
	};

	template<SerializeFormat format>
	concept HasReader = !IsVoid<typename FormatBind<format>::Reader>;

	template<SerializeFormat format>
	concept HasWriter = !IsVoid<typename FormatBind<format>::Writer>;
}    // namespace p
