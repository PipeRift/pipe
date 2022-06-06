// Copyright 2015-2022 Piperift - All rights reserved

#include "PSerl/ReadWriter.h"

#include "PCore/Checks.h"


namespace p
{
	void ReadWriter::BeginObject()
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

	bool ReadWriter::EnterNext(StringView name)
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

	void ReadWriter::BeginArray(u32& size)
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

	bool ReadWriter::EnterNext()
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

	void ReadWriter::Leave()
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

	void ReadWriter::PushAddFlags(WriteFlags flags)
	{
		if (IsWriting())
		{
			GetWrite().PushAddFlags(flags);
		}
	}

	void ReadWriter::PushRemoveFlags(WriteFlags flags)
	{
		if (IsWriting())
		{
			GetWrite().PushRemoveFlags(flags);
		}
	}

	void ReadWriter::PopFlags()
	{
		if (IsWriting())
		{
			GetWrite().PopFlags();
		}
	}
}    // namespace p
