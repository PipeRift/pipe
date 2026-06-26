// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "PipePlatform.h"


// Forward declarations of all serialization
namespace p
{
	enum WriteFlags : sizet;

	struct Reader;
	struct Writer;

	struct IFormatReader;
	struct IFormatWriter;

	struct ReadWriter;

	struct JsonFormatReader;
	struct JsonFormatWriter;

	struct BinaryFormatReader;
	struct BinaryFormatWriter;
}    // namespace p
