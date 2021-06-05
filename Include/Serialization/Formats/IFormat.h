// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"
#include "Serialization/SerializationTypes.h"
#include "TypeTraits.h"


namespace Rift::Serl
{
	struct IFormatReader
	{};

	enum WriteFlags
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
		void PushAddFlags(WriteFlags flags)
		{
			flagStack.Add(activeFlags);
			activeFlags |= flags;
		}
		void PushRemoveFlags(WriteFlags flags)
		{
			flagStack.Add(activeFlags);
			activeFlags &= ~flags;
		}
		void PopFlags()
		{
			activeFlags = flagStack.Last();
			flagStack.RemoveAt(flagStack.Size() - 1);
		}

		sizet GetFlags() const
		{
			return activeFlags;
		}
	};


	template <Format format>
	struct FormatBind
	{
		using Reader = void;
		using Writer = void;
	};

	template <Format format>
	concept HasReader = !IsVoid<typename FormatBind<format>::Reader>;

	template <Format format>
	concept HasWriter = !IsVoid<typename FormatBind<format>::Writer>;
}    // namespace Rift::Serl
