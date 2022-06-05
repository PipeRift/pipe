// Copyright 2015-2022 Piperift - All rights reserved

#include "Serialization/Contexts/CommonContext.h"

#include "Core/Checks.h"


namespace pipe::Serl
{
	void CommonContext::BeginObject()
	{
		if (IsWriting())
		{
			writeContext->BeginObject();
		}
		else
		{
			readContext->BeginObject();
		}
	}

	bool CommonContext::EnterNext(StringView name)
	{
		if (IsWriting())
		{
			return writeContext->EnterNext(name);
		}
		else
		{
			return readContext->EnterNext(name);
		}
	}

	void CommonContext::BeginArray(u32& size)
	{
		if (IsWriting())
		{
			writeContext->BeginArray(size);
		}
		else
		{
			readContext->BeginArray(size);
		}
	}

	bool CommonContext::EnterNext()
	{
		if (IsWriting())
		{
			return writeContext->EnterNext();
		}
		else
		{
			return readContext->EnterNext();
		}
	}

	void CommonContext::Leave()
	{
		if (IsWriting())
		{
			writeContext->Leave();
		}
		else
		{
			readContext->Leave();
		}
	}

	void CommonContext::PushAddFlags(WriteFlags flags)
	{
		if (IsWriting())
		{
			GetWrite().PushAddFlags(flags);
		}
	}

	void CommonContext::PushRemoveFlags(WriteFlags flags)
	{
		if (IsWriting())
		{
			GetWrite().PushRemoveFlags(flags);
		}
	}

	void CommonContext::PopFlags()
	{
		if (IsWriting())
		{
			GetWrite().PopFlags();
		}
	}
}    // namespace pipe::Serl
