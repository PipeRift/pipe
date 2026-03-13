// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "PipePlatform.h"


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
	struct IPool;
	template<typename T>
	struct TPool;
	struct PoolInstance;


	struct SortLessStatics;
	struct IdContext;

	template<typename... T>
	struct TTypeList;    // TODO: Move to a TemplatesFwd header

	struct IdScope;

	template<typename... T>
	struct TIdScope;
	template<typename... T>
	using TIdScopeRef = const TIdScope<T...>&;

	struct CChild;
	struct CParent;
}    // namespace p::ecs
