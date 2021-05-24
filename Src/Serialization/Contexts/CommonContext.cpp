// Copyright 2015-2021 Piperift - All rights reserved

#include "Misc/Checks.h"
#include "Serialization/Contexts/CommonContext.h"


namespace Rift::Serl
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
}    // namespace Rift::Serl