// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "PRefl/Builders/DataTypeBuilder.h"
#include "PRefl/Builders/EnumTypeBuilder.h"
#include "PRefl/Builders/NativeTypeBuilder.h"
#include "PRefl/Builders/StaticInitializers.h"
#include "PRefl/CoreTypes.h"
#include "PRefl/GetType.h"
#include "PRefl/HasType.h"
#include "PRefl/ReflectionFlags.h"
#include "PRefl/ReflectionTraits.h"


/**
 * There are 2 types of reflected data structures:
 *
 * - STRUCT: Inheritance. Not virtual. Inlined in memory
 * - CLASS: Inheritance. Virtual. Smart creation and destruction. Instanced in memory.
 */
