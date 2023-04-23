// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Span.h"
#include "Pipe/ECS/Access.h"
#include "Pipe/ECS/Components/CChild.h"
#include "Pipe/ECS/Components/CParent.h"
#include "Pipe/ECS/Id.h"


namespace p::ecs
{
	// Link a list of nodes at the end of the parent children list
	PIPE_API void AddChildren(
	    TAccessRef<TWrite<CChild>, TWrite<CParent>> access, Id node, TSpan<const Id> children);
	// Link a list of nodes after prevChild in the list of children nodes
	PIPE_API void AddChildrenAfter(TAccessRef<TWrite<CChild>, TWrite<CParent>> access, Id node,
	    TSpan<Id> children, Id prevChild);
	PIPE_API void TransferChildren(TAccessRef<TWrite<CChild>, TWrite<CParent>> access,
	    TSpan<const Id> children, Id destination);
	// TODO: void TransferAllChildren(Tree& ast, Id origin, Id destination);
	PIPE_API void RemoveChildren(TAccessRef<TWrite<CParent>, TWrite<CChild>> access,
	    TSpan<const Id> children, bool keepComponents);
	PIPE_API void RemoveAllChildren(TAccessRef<TWrite<CParent>, TWrite<CChild>> access,
	    TSpan<const Id> parents, bool keepComponents = false);

	PIPE_API TArray<Id>* GetMutChildren(TAccessRef<TWrite<CParent>> access, Id node);
	PIPE_API const TArray<Id>* GetChildren(TAccessRef<CParent> access, Id node);
	PIPE_API void GetChildren(
	    TAccessRef<CParent> access, TSpan<const Id> nodes, TArray<Id>& outLinkedNodes);
	/**
	 * Finds all nodes connected recursively.
	 */
	PIPE_API void GetChildrenDeep(TAccessRef<CParent> access, TSpan<const Id> roots,
	    TArray<Id>& outLinkedNodes, u32 depth = 0);
	PIPE_API Id GetParent(TAccessRef<CChild> access, Id node);
	PIPE_API void GetParents(
	    TAccessRef<CChild> access, TSpan<const Id> children, TArray<Id>& outParents);
	PIPE_API void GetAllParents(TAccessRef<CChild> access, Id node, TArray<Id>& outParents);
	PIPE_API void GetAllParents(
	    TAccessRef<CChild> access, TSpan<const Id> childrenIds, TArray<Id>& outParents);

	/**
	 * Find a parent id matching a delegate
	 */
	PIPE_API Id FindParent(
	    TAccessRef<CChild> access, Id child, const TFunction<bool(Id)>& callback);
	PIPE_API void FindParents(TAccessRef<CChild> access, TSpan<const Id> children,
	    TArray<Id>& outParents, const TFunction<bool(Id)>& callback);

	// void Copy(Tree& ast, t TArray<Id>& nodes, TArray<Id>& outNewNodes);
	// void CopyDeep(Tree& ast, const TArray<Id>& rootNodes, TArray<Id>& outNewRootNodes);
	// void CopyAndTransferAllChildrenDeep(Tree& ast, Id root, Id otherRoot);

	PIPE_API void Remove(TAccessRef<TWrite<CChild>, TWrite<CParent>> access, TSpan<Id> nodes);
	PIPE_API void RemoveDeep(TAccessRef<TWrite<CChild>, TWrite<CParent>> access, TSpan<Id> nodes);

	/**
	 * Iterates children nodes making sure child->parent links are correct or fixed
	 * Only first depth links are affected
	 * Complexity: O(N)
	 * @parents: where to look for children to fix up
	 * @return true if an incorrect link was found and fixed
	 */
	PIPE_API bool FixParentLinks(TAccessRef<TWrite<CChild>, CParent> access, TSpan<Id> parents);

	/**
	 * Iterates children nodes looking for invalid child->parent links
	 * Only first depth links are affected
	 * Complexity: O(1) <-> O(N) (First invalid link makes an early out)
	 * @parents: where to look for children
	 * @return true if an incorrect link was found
	 */
	PIPE_API bool ValidateParentLinks(TAccessRef<CChild, CParent> access, TSpan<Id> parents);

	PIPE_API void GetRoots(TAccessRef<CChild, CParent> access, TArray<Id>& outRoots);
}    // namespace p::ecs
