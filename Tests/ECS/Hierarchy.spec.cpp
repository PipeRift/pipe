// Copyright 2015-2026 Piperift. All Rights Reserved.


#include <PipeTest.h>
#include <PipeECS.h>


using namespace p;


namespace
{
	IdContext ctx;
	Id root;
	Id child1;
	Id child2;
	Id child3;
	Id grandchild;
}    // namespace


namespace
{
// Auto-registers via static init (macro-free go_bandit equivalent).
const bool autoRegistered = []()
{
Spec("ECS.Hierarchy", []()
{
	BeforeEach([]()
	{
		ctx        = {};
		root       = AddId(ctx);
		child1     = AddId(ctx);
		child2     = AddId(ctx);
		child3     = AddId(ctx);
		grandchild = AddId(ctx);
	});

	Describe("AttachId", []()
	{
		It("Creates bidirectional parent-child link for single child", []()
		{
			AttachId({ctx}, root, child1);

			Expect(ctx.Has<CParent>(root)).ToBeTrue();
			Expect(ctx.Has<CChild>(child1)).ToBeTrue();
			Expect(ctx.Get<CParent>(root).children.Size()).ToEqual(1);
			Expect(ctx.Get<CParent>(root).children[0]).ToEqual(child1);
			Expect(ctx.Get<CChild>(child1).parent).ToEqual(root);
		});

		It("Appends multiple children to same parent", []()
		{
			AttachId({ctx}, root, {child1, child2, child3});

			Expect(ctx.Has<CParent>(root)).ToBeTrue();
			Expect(ctx.Has<CChild>(child1)).ToBeTrue();
			Expect(ctx.Has<CChild>(child2)).ToBeTrue();
			Expect(ctx.Has<CChild>(child3)).ToBeTrue();
			Expect(ctx.Get<CParent>(root).children.Size()).ToEqual(3);
			Expect(ctx.Get<CParent>(root).children[0]).ToEqual(child1);
			Expect(ctx.Get<CParent>(root).children[1]).ToEqual(child2);
			Expect(ctx.Get<CParent>(root).children[2]).ToEqual(child3);
			Expect(ctx.Get<CChild>(child1).parent).ToEqual(root);
			Expect(ctx.Get<CChild>(child2).parent).ToEqual(root);
			Expect(ctx.Get<CChild>(child3).parent).ToEqual(root);
		});
	});

	Describe("AttachIdAfter", []()
	{
		It("Inserts child after specified sibling preserving order", []()
		{
			AttachId({ctx}, root, {child1, child3});
			AttachIdAfter({ctx}, root, child2, child1);

			Expect(ctx.Get<CParent>(root).children.Size()).ToEqual(3);
			Expect(ctx.Get<CParent>(root).children.FindIndex(child2)).ToEqual(1);
		});
	});

	Describe("TransferIdChildren", []()
	{
		It("Moves children from old parent to new parent", []()
		{
			Id newRoot = AddId(ctx);
			AttachId({ctx}, root, {child1, child2});
			TransferIdChildren({ctx}, {child1, child2}, newRoot);

			Expect(ctx.Get<CParent>(root).children.IsEmpty()).ToBeTrue();
			Expect(ctx.Has<CParent>(newRoot)).ToBeTrue();
			Expect(ctx.Get<CParent>(newRoot).children.Size()).ToEqual(2);
			Expect(ctx.Get<CChild>(child1).parent).ToEqual(newRoot);
			Expect(ctx.Get<CChild>(child2).parent).ToEqual(newRoot);
		});
	});

	Describe("DetachIdParent", []()
	{
		BeforeEach([]()
		{
			AttachId({ctx}, root, {child1, child2});
		});

		It("Retains CChild component when keepComponents is true", []()
		{
			DetachIdParent({ctx}, child1, true);

			Expect(ctx.Has<CChild>(child1)).ToBeTrue();
			Expect(ctx.Get<CChild>(child1).parent).ToEqual(NoId);
			Expect(ctx.Get<CParent>(root).children.Size()).ToEqual(1);
		});

		It("Removes CChild from detached child and removes from parent list", []()
		{
			DetachIdParent({ctx}, child1, false);

			Expect(ctx.Has<CChild>(child1)).ToBeFalse();
			Expect(ctx.Get<CParent>(root).children.Contains(child1)).ToBeFalse();
		});

		It("Removes empty CParent when all children are detached", []()
		{
			DetachIdParent({ctx}, {child1, child2}, false);

			Expect(ctx.Has<CChild>(child1)).ToBeFalse();
			Expect(ctx.Has<CChild>(child2)).ToBeFalse();
			Expect(ctx.Has<CParent>(root)).ToBeFalse();
		});
	});

	Describe("DetachIdChildren", []()
	{
		BeforeEach([]()
		{
			AttachId({ctx}, root, {child1, child2});
		});

		It("Severes all children but retains CChild when keepComponents is true", []()
		{
			DetachIdChildren({ctx}, root, true);

			Expect(ctx.Has<CChild>(child1)).ToBeTrue();
			Expect(ctx.Has<CChild>(child2)).ToBeTrue();
			Expect(ctx.Get<CChild>(child1).parent).ToEqual(NoId);
			Expect(ctx.Get<CChild>(child2).parent).ToEqual(NoId);
			Expect(ctx.Get<CParent>(root).children.IsEmpty()).ToBeTrue();
		});

		It("Removes CChild and CParent when keepComponents is false", []()
		{
			DetachIdChildren({ctx}, root, false);

			Expect(ctx.Has<CChild>(child1)).ToBeFalse();
			Expect(ctx.Has<CChild>(child2)).ToBeFalse();
			Expect(ctx.Has<CParent>(root)).ToBeFalse();
		});
	});

	Describe("GetIdChildren", []()
	{
		BeforeEach([]()
		{
			AttachId({ctx}, root, {child1, child2});
			AttachId({ctx}, child1, grandchild);
		});

		It("Returns child list for parent entities", []()
		{
			const auto* children = GetIdChildren({ctx}, root);
			Expect(children).ToNotEqual(nullptr);
			Expect(children->Size()).ToEqual(2);
			Expect(children->Contains(child1)).ToBeTrue();
			Expect(children->Contains(child2)).ToBeTrue();
		});

		It("Combines children from multiple parents into one list", []()
		{
			TArray<Id> outChildren;
			GetIdChildren({ctx}, {root, child1}, outChildren);
			Expect(outChildren.Size()).ToEqual(3);
			Expect(outChildren.Contains(grandchild)).ToBeTrue();
		});

		It("Returns null for entities without CParent component", []()
		{
			Expect(GetIdChildren({ctx}, child2)).ToEqual(nullptr);
		});
	});

	Describe("GetAllIdChildren", []()
	{
		BeforeEach([]()
		{
			AttachId({ctx}, root, child1);
			AttachId({ctx}, child1, grandchild);
		});

		It("Recurses full tree depth to collect all descendents", []()
		{
			TArray<Id> outChildren;
			GetAllIdChildren({ctx}, root, outChildren, 10);
			Expect(outChildren.Size()).ToEqual(2);
			Expect(outChildren.Contains(grandchild)).ToBeTrue();
		});

		It("Respects depth limit to return only immediate children", []()
		{
			TArray<Id> outChildren;
			GetAllIdChildren({ctx}, root, outChildren, 1);
			Expect(outChildren.Size()).ToEqual(1);
			Expect(outChildren.Contains(grandchild)).ToBeFalse();
		});
	});

	Describe("GetIdParent", []()
	{
		BeforeEach([]()
		{
			AttachId({ctx}, root, child1);
			AttachId({ctx}, child1, grandchild);
		});

		It("Returns parent Id for child entities", []()
		{
			Expect(GetIdParent({ctx}, child1)).ToEqual(root);
			Expect(GetIdParent({ctx}, grandchild)).ToEqual(child1);
		});

		It("Returns unique parents for multiple children", []()
		{
			TArray<Id> outParents;
			GetIdParent({ctx}, {child1, grandchild}, outParents);
			Expect(outParents.Size()).ToEqual(2);
			Expect(outParents.Contains(root)).ToBeTrue();
			Expect(outParents.Contains(child1)).ToBeTrue();
		});

		It("Returns NoId for root entities without parent", []()
		{
			Expect(GetIdParent({ctx}, root)).ToEqual(NoId);
		});

		It("Returns NoId for entities without CChild component", []()
		{
			Expect(GetIdParent({ctx}, child2)).ToEqual(NoId);
		});
	});

	Describe("GetAllIdParents", []()
	{
		BeforeEach([]()
		{
			AttachId({ctx}, root, child1);
			AttachId({ctx}, child1, grandchild);
		});

		It("Traverses full ancestry chain from leaf to root", []()
		{
			TArray<Id> outParents;
			GetAllIdParents({ctx}, grandchild, outParents);
			Expect(outParents.Size()).ToEqual(2);
			Expect(outParents[0]).ToEqual(child1);
			Expect(outParents[1]).ToEqual(root);
		});

		It("Returns empty when entity has no CChild component", []()
		{
			TArray<Id> outParents;
			GetAllIdParents({ctx}, child2, outParents);
			Expect(outParents.IsEmpty()).ToBeTrue();
		});
	});

	Describe("FindIdParent", []()
	{
		BeforeEach([]()
		{
			AttachId({ctx}, root, child1);
			AttachId({ctx}, child1, grandchild);
		});

		It("Finds ancestor two levels up matching predicate", []()
		{
			Expect(FindIdParent({ctx}, grandchild,
			               [&](Id id)
			{
				return id == root;
			})).ToEqual(root);
		});

		It("Finds immediate parent matching predicate", []()
		{
			Expect(FindIdParent({ctx}, grandchild,
			               [&](Id id)
			{
				return id == child1;
			})).ToEqual(child1);
		});

		It("Returns NoId when no ancestor matches predicate", []()
		{
			Expect(IsNone(FindIdParent({ctx}, grandchild,
			               [](Id)
			{
				return false;
			}))).ToBeTrue();
		});
	});

	Describe("FindIdParents", []()
	{
		It("Finds nearest matching ancestor for deep entity", []()
		{
			Id intermediate = AddId(ctx);
			AttachId({ctx}, root, child1);
			AttachId({ctx}, child1, intermediate);
			AttachId({ctx}, intermediate, grandchild);

			TArray<Id> outParents;
			FindIdParents({ctx}, grandchild, outParents, [](Id)
			{
				return true;
			});
			Expect(outParents.Size()).ToEqual(1);
			Expect(outParents.Contains(intermediate)).ToBeTrue();
		});

		It("Returns empty when no ancestor matches predicate", []()
		{
			TArray<Id> outParents;
			FindIdParents({ctx}, child1, outParents, [](Id)
			{
				return false;
			});
			Expect(outParents.IsEmpty()).ToBeTrue();
		});
	});

	Describe("GetIdRoots", []()
	{
		It("Returns empty when no hierarchy exists", []()
		{
			TArray<Id> roots;
			GetIdRoots({ctx}, roots);
			Expect(roots.IsEmpty()).ToBeTrue();
		});

		It("Finds root of single-parent hierarchy", []()
		{
			AttachId({ctx}, root, {child1, child2});

			TArray<Id> roots;
			GetIdRoots({ctx}, roots);
			Expect(roots.Size()).ToEqual(1);
			Expect(roots.Contains(root)).ToBeTrue();
		});

		It("Returns multiple roots from independent trees", []()
		{
			Id root2 = AddId(ctx);
			AttachId({ctx}, root, {child1, child2});
			AttachId({ctx}, root2, child3);

			TArray<Id> roots;
			GetIdRoots({ctx}, roots);
			Expect(roots.Size()).ToEqual(2);
			Expect(roots.Contains(root)).ToBeTrue();
			Expect(roots.Contains(root2)).ToBeTrue();
		});

		It("Excludes entities that are both parent and child of someone", []()
		{
			AttachId({ctx}, root, child1);
			AttachId({ctx}, child1, grandchild);

			TArray<Id> roots;
			GetIdRoots({ctx}, roots);
			Expect(roots.Size()).ToEqual(1);
			Expect(roots.Contains(root)).ToBeTrue();
			Expect(roots.Contains(child1)).ToBeFalse();
		});
	});

	Describe("GetIdParentRoots", []()
	{
		BeforeEach([]()
		{
			AttachId({ctx}, root, child1);
			AttachId({ctx}, child1, grandchild);
		});

		It("Walks child chain up to root ancestor", []()
		{
			TArray<Id> roots;
			GetIdParentRoots({ctx}, grandchild, roots, false);
			Expect(roots.Size()).ToEqual(1);
			Expect(roots.Contains(root)).ToBeTrue();
		});

		It("Handles children from different trees", []()
		{
			Id root2    = AddId(ctx);
			Id childOf2 = AddId(ctx);
			AttachId({ctx}, root2, childOf2);

			TArray<Id> roots;
			GetIdParentRoots({ctx}, {grandchild, childOf2}, roots, false);
			Expect(roots.Size()).ToEqual(2);
			Expect(roots.Contains(root)).ToBeTrue();
			Expect(roots.Contains(root2)).ToBeTrue();
		});

		It("Considers input entities as roots when considerChildren flag is set", []()
		{
			TArray<Id> roots;
			GetIdParentRoots({ctx}, {root, grandchild}, roots, true);
			Expect(roots.Size()).ToEqual(1);
			Expect(roots.Contains(root)).ToBeTrue();
		});

		It("Returns empty for empty input", []()
		{
			TArray<Id> roots;
			GetIdParentRoots({ctx}, {}, roots, false);
			Expect(roots.IsEmpty()).ToBeTrue();
		});

		It("Returns empty for entities with no parent", []()
		{
			TArray<Id> roots;
			GetIdParentRoots({ctx}, child2, roots, false);
			Expect(roots.IsEmpty()).ToBeTrue();
		});
	});

	Describe("FixParentIdLinks", []()
	{
		BeforeEach([]()
		{
			AttachId({ctx}, root, child1);
		});

		It("Returns false when parent-child links are already correct", []()
		{
			Expect(FixParentIdLinks({ctx}, root)).ToBeFalse();
		});

		It("Fixes child->parent reference when it does not match parent's list", []()
		{
			ctx.Get<CChild>(child1).parent = NoId;

			Expect(FixParentIdLinks({ctx}, root)).ToBeTrue();
			Expect(ctx.Get<CChild>(child1).parent).ToEqual(root);
		});

		It("Adds missing CChild component to orphan children", []()
		{
			ctx.Remove<CChild>(child1);
			Expect(ctx.Has<CChild>(child1)).ToBeFalse();

			Expect(FixParentIdLinks({ctx}, root)).ToBeTrue();
			Expect(ctx.Has<CChild>(child1)).ToBeTrue();
			Expect(ctx.Get<CChild>(child1).parent).ToEqual(root);
		});
	});

	Describe("ValidateParentIdLinks", []()
	{
		BeforeEach([]()
		{
			AttachId({ctx}, root, child1);
		});

		It("Returns true when all parent-child links are consistent", []()
		{
			Expect(ValidateParentIdLinks({ctx}, root)).ToBeTrue();
		});

		It("Returns false when child->parent reference is mismatched", []()
		{
			ctx.Get<CChild>(child1).parent = NoId;

			Expect(ValidateParentIdLinks({ctx}, root)).ToBeFalse();
		});

		It("Returns false when CChild component is missing from child", []()
		{
			ctx.Remove<CChild>(child1);

			Expect(ValidateParentIdLinks({ctx}, root)).ToBeFalse();
		});
	});
});
return true;
}();
}    // namespace
