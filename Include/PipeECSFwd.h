// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "PipeContainersFwd.h"
#include "PipePlatform.h"


#ifndef P_ID_IS_64BIT
	#define P_ID_IS_64BIT 0
#endif


namespace p
{
	////////////////////////////////
	// FORWARD DECLARATIONS
	//

	enum class RmIdFlags : u8;

	struct Id;

	template<typename T>
	struct CMdfdWithLast;

	struct CMdfdWithoutLast;

	template<typename T>
	struct CMdfd;

	struct CRemoved;

	struct CParent;
	struct CChild;

	struct CNotSerialized;

	enum class IdRemovePolicy : u8;

	enum class PoolRemovePolicy : u8;

	struct SortLessStatics;
	struct IdContext;

	template<typename Parent>
	struct TIdOperations;

	struct IdRegistry;

	class EntityReader;
	class EntityWriter;

	struct PoolIterator;
	struct IPool;
	template<typename T>
	struct TPool;
	struct ComponentPool;
	struct PoolInstance;

	template<typename... T>
	struct TTypeList;

	struct IdScope;

	template<typename T>
	struct TIsAutoModified;

	template<typename W, typename... R>
	struct TIdScopeBase;

	template<typename... T>
	struct Writes;

	template<typename... T>
	struct TIdScope;
	template<typename... T>
	using TIdScopeRef = const TIdScope<T...>&;
}    // namespace p
