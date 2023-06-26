// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"


namespace p::ecs
{
	enum class Id : u32;

	template<typename Type>
	struct IdTraits;

	struct IdRegistry;

	class EntityReader;
	class EntityWriter;

	enum class PoolDeletionPolicy : u8;
	struct PoolIterator;
	struct BasePool;
	template<typename T, typename Allocator = ArenaAllocator>
	struct TPool;
	struct PoolInstance;


	struct SortLessStatics;
	struct EntityContext;

	enum class AccessMode : u8;
	struct TypeAccess;
	template<typename T, AccessMode inMode>
	struct TTypeAccess;
	template<typename T>
	struct TRead;
	template<typename T>
	struct TWrite;
	template<typename T>
	struct TTypeAccessInfo;
	template<typename... T>
	struct TAccess;
	template<typename... T>
	using TAccessRef = const TAccess<T...>&;
	struct Access;

	struct CChild;
	struct CParent;
}    // namespace p::ecs
