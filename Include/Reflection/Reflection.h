// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Builders/DataTypeBuilder.h"
#include "Reflection/Builders/EnumTypeBuilder.h"
#include "Reflection/Builders/NativeTypeBuilder.h"
#include "Reflection/Builders/StaticInitializers.h"
#include "Reflection/CoreTypes.h"
#include "Reflection/GetType.h"
#include "Reflection/HasType.h"
#include "Reflection/ReflectionFlags.h"
#include "Reflection/ReflectionTraits.h"


/**
 * There are 2 types of reflected data structures:
 *
 * - STRUCT: Inheritance. Not virtual. Inlined in memory
 * - CLASS: Inheritance. Virtual. Smart creation and destruction. Instanced in memory.
 */
