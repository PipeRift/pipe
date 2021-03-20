// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/GetType.h"
#include "Reflection/HasType.h"
#include "Reflection/PredefinedTypes.h"
#include "Reflection/ReflectionTags.h"
#include "Reflection/Registry/DataTypeBuilder.h"
#include "Reflection/Registry/EnumTypeBuilder.h"
#include "Reflection/Registry/NativeTypeBuilder.h"


/**
 * There are 2 types of reflected data structures:
 *
 * - STRUCT: Inheritance. Not virtual. Inlined in memory
 * - CLASS: Inheritance. Virtual. Smart creation and destruction. Instanced in memory.
 */
