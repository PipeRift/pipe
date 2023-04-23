// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Reflect/Builders/DataTypeBuilder.h"
#include "Pipe/Reflect/Builders/EnumTypeBuilder.h"
#include "Pipe/Reflect/Builders/NativeTypeBuilder.h"
#include "Pipe/Reflect/Builders/StaticInitializers.h"
#include "Pipe/Reflect/CoreTypes.h"
#include "Pipe/Reflect/GetType.h"
#include "Pipe/Reflect/HasType.h"
#include "Pipe/Reflect/ReflectionFlags.h"
#include "Pipe/Reflect/ReflectionTraits.h"


/**
 * There are 2 types of reflected data structures:
 *
 * - STRUCT: Inheritance. Not virtual. Inlined in memory
 * - CLASS: Inheritance. Virtual. Smart creation and destruction. Instanced in memory.
 */
