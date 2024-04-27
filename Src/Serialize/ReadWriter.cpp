// Copyright 2015-2024 Piperift - All rights reserved

#include "Pipe/Serialize/ReadWriter.h"

#include "Pipe/Core/Checks.h"


namespace p
{
	void ReadWriter::BeginObject()
	{
		if (IsWriting())
		{
			writer->BeginObject();
		}
		else
		{
			reader->BeginObject();
		}
	}

	bool ReadWriter::EnterNext(StringView name)
	{
		if (IsWriting())
		{
			return writer->EnterNext(name);
		}
		else
		{
			return reader->EnterNext(name);
		}
	}

	void ReadWriter::BeginArray(u32& size)
	{
		if (IsWriting())
		{
			writer->BeginArray(size);
		}
		else
		{
			reader->BeginArray(size);
		}
	}

	bool ReadWriter::EnterNext()
	{
		if (IsWriting())
		{
			return writer->EnterNext();
		}
		else
		{
			return reader->EnterNext();
		}
	}

	void ReadWriter::Leave()
	{
		if (IsWriting())
		{
			writer->Leave();
		}
		else
		{
			reader->Leave();
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
